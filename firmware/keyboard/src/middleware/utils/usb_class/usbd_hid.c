/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
 * Description: LiteOS USB Driver HID Data Stream
 * Author: Huawei LiteOS Team
 * Create: 2019-10-24
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --------------------------------------------------------------------------- */

#include "gadget/usbd_hid.h"
#ifdef CONFIG_COMPAT_LINUX
#include <poll.h>
#endif
#ifdef CONFIG_DRIVERS_USB_HID_VFS_INTERFACE
#include <fs/fs.h>
#endif
#include "errcode.h"

extern uint8_t g_get_report_data[CONFIG_DRIVERS_USB_HID_INPUT_REPORT_LEN];

#if defined(CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT) && defined(CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT_EVENT)
static ssize_t hid_read_data(struct hid_dev_s *hid, uint8_t report_index, char *buf, size_t buflen);
#endif
static ssize_t hid_write_data(struct hid_dev_s *hid, uint8_t report_index, const char *buffer, size_t buflen);

KAL_SPIN_LOCK_INIT(g_hid_mode_lock);
__attribute__((always_inline)) static inline uint32_t hid_acquire_lock(void)
{
  uint32_t flags;
  KAL_SpinLockSave((KalSpinLock *)&g_hid_mode_lock, &flags);
  return flags;
}

__attribute__((always_inline)) static inline void hid_release_lock(uint32_t flags)
{
  KAL_SpinUnlockRestore((KalSpinLock *)&g_hid_mode_lock, flags);
}

#define HID_FUNC_CNT_INVALID 0xFFFFFFFF
static uint32_t g_hid_ops_cnt = HID_FUNC_CNT_INVALID;

#ifdef CONFIG_DRIVERS_USB_HID_FUNC_INTERFACE
static struct hid_dev_s *g_hid_dev = NULL;
#endif

#ifdef CONFIG_DRIVERS_USB_HID_VFS_INTERFACE
static int hid_open(FAR struct file *filep);
static int hid_close(FAR struct file *filep);
static ssize_t hid_write(FAR struct file *filep, FAR const char *buffer, size_t buflen);
#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT
static ssize_t hid_read(FAR struct file *filep, FAR char *buffer, size_t buflen);
static int hid_poll(FAR struct file *filep, poll_table *fds);
#endif

static const struct file_operations_vfs g_hid_fops =
{
  .open   = hid_open,
  .close  = hid_close,
#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT
  .read   = hid_read,
#else
  .read   = NULL,
#endif
  .write  = hid_write,
  .seek   = NULL,
  .ioctl  = NULL,
#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT
  .poll   = hid_poll,
#else
  .poll   = NULL,
#endif
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
  .unlink = NULL,
#endif
};

static int hid_open(FAR struct file *filep)
{
  struct hid_dev_s *hid;
  uint32_t flags;

  hid = (struct hid_dev_s *)filep->f_inode->i_private;
  if (hid == NULL)
    {
      usb_err("Invalid private parameter!\n");
      return ERRCODE_INVALID_PARAM;
    }

  flags = hid_acquire_lock();
  if (g_hid_ops_cnt)
    {
      hid_release_lock(flags);
      usb_warn("hid device is opened!\n");
      return ERRCODE_USB_HID_DEVICE_OPENED;
    }
  g_hid_ops_cnt++;
  hid_release_lock(flags);

  return 0;
}

static int hid_close(FAR struct file *filep)
{
  struct hid_dev_s *hid;
  uint32_t flags;

  hid = (struct hid_dev_s *)filep->f_inode->i_private;
  if (hid == NULL)
    {
      usb_err("Invalid private parameter!\n");
      return ERRCODE_INVALID_PARAM;
    }

  flags = hid_acquire_lock();
  g_hid_ops_cnt--;
  hid_release_lock(flags);

  return 0;
}

static ssize_t hid_write(FAR struct file *filep, FAR const char *buffer, size_t buflen)
{
  struct hid_dev_s *hid = (struct hid_dev_s *)filep->f_inode->i_private;

  return hid_write_data(hid, 0, buffer, buflen);
}

#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT
static ssize_t hid_read(FAR struct file *filep, FAR char *buffer, size_t buflen)
{
  struct hid_dev_s *hid = (struct hid_dev_s *)filep->f_inode->i_private;

  return hid_read_data(hid, 0, buffer, buflen);
}

static int hid_poll(FAR struct file *filep, poll_table *fds)
{
  struct hid_dev_s *hid;
  uint32_t revents = 0;
  uint32_t ret;
  uint32_t value = fds->key & (POLLIN | POLLRDNORM);

  hid = (struct hid_dev_s *)filep->f_inode->i_private;
  if (hid == NULL || hid->connected == 0)
    {
      return (POLLERR);
    }

  if (value)
    {
      ret = LOS_EventPoll(&hid->hid_data[0].read_event.uwEventID, USB_HID_READ_EVENT, LOS_WAITMODE_OR);
      if (ret == USB_HID_READ_EVENT)
        {
          revents |= value;
        }
    }

  return (int)revents;
}
#endif /* CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT */
#endif /* CONFIG_DRIVERS_USB_HID_VFS_INTERFACE */

bool hid_is_running(void)
{
  bool ret = true;
  uint32_t flags;

  flags = hid_acquire_lock();
  if (!(g_hid_ops_cnt > 0 && g_hid_ops_cnt < HID_FUNC_CNT_INVALID))
    {
      ret = false;
      g_hid_ops_cnt = HID_FUNC_CNT_INVALID;
    }
  hid_release_lock(flags);

  return ret;
}

#if defined(CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT) && defined(CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT_EVENT)
static ssize_t hid_read_data(struct hid_dev_s *hid, uint8_t report_index, char *buf, size_t buflen)
{
  struct hid_data_ctl *hid_data;
  uint32_t flags;
  uint32_t ret_event;
  errno_t ret;
  ssize_t len;

  if (buf == NULL || buflen == 0 || buflen > HID_OUT_DATA_SIZE)
    {
      usb_err("invalid buf or invalid buflen. buflen: %u, max size: %u\n", buflen, HID_OUT_DATA_SIZE);
      return ERRCODE_INVALID_PARAM;
    }

  if (hid == NULL || hid->connected == 0)
    {
      usb_warn("invalid hid or hid is disconnected!\n");
      return ERRCODE_INVALID_PARAM;
    }

  if (report_index >= HID_REPORT_MAP_NUM || hid->hid_data[report_index].read_buf == NULL)
    {
      return ERRCODE_INVALID_PARAM;
    }
  hid_data = &hid->hid_data[report_index];

  ret_event = LOS_EventRead(&hid_data->read_event, USB_HID_READ_EVENT | USB_HID_EXIT_EVENT,
                            LOS_WAITMODE_OR, LOS_WAIT_FOREVER);
  if (ret_event & USB_HID_EXIT_EVENT)
    {
      return ERRCODE_USB_HID_EXIT_EVENT;
    }
  (void)LOS_EventClear(&hid_data->read_event, ~USB_HID_READ_EVENT);

  KAL_SpinLockSave((KalSpinLock *)&hid_data->hid_lock, &flags);
  ret = memcpy_s(buf, buflen, hid_data->read_buf, hid_data->read_len);
  if (ret != EOK)
    {
      KAL_SpinUnlockRestore((KalSpinLock *)&hid_data->hid_lock, flags);
      usb_err("memcpy failed! ret = %d\n", ret);
      return ERRCODE_USB_HID_MEMCPY_FAIL;
    }
  len = hid_data->read_len;
  hid_data->read_len = 0;
  KAL_SpinUnlockRestore((KalSpinLock *)&hid_data->hid_lock, flags);

  return len;
}
#endif

void hid_submit_data(struct hid_data_ctl *hid_data)
{
  struct usbdev_req_s *req = &hid_data->inputreq;

  req->buf = hid_data->write_buf[hid_data->write_head];
  req->len = hid_data->write_len[hid_data->write_head];
  hid_data->write_busy = 1;

  (void)EP_SUBMIT(hid_data->in_ep, req);
}

static ssize_t hid_write_data(struct hid_dev_s *hid, uint8_t report_index, const char *buf, size_t buflen)
{
  struct hid_data_ctl *hid_data;
  uint32_t tail_temp;
  uint32_t flags;
  errno_t err;
  ssize_t ret = ERRCODE_FAIL;

  if (hid == NULL || hid->connected == 0)
    {
      usb_warn("invalid hid or hid is disconnected!\n");
      return ERRCODE_INVALID_PARAM;
    }

  if (report_index >= HID_REPORT_MAP_NUM || hid->hid_data[report_index].in_ep_enabled == false)
    {
      return ERRCODE_INVALID_PARAM;
    }

  hid_data = &hid->hid_data[report_index];
  if (buf == NULL || buflen == 0 || buflen > hid_data->write_buf_len)
    {
      usb_err("invalid buf or invalid buflen!\n");
      return ERRCODE_INVALID_PARAM;
    }

  KAL_SpinLockSave((KalSpinLock *)&hid_data->hid_lock, &flags);
  tail_temp = hid_data->write_tail;
  if ((tail_temp + 1) % HID_WRITE_BUF_NUM == hid_data->write_head)
    {
      goto done;
    }

  err = memcpy_s(hid_data->write_buf[tail_temp], hid_data->write_buf_len, buf, buflen);
  if (err != EOK)
    {
      goto done;
    }

  hid_data->write_len[tail_temp] = buflen;
  hid_data->write_tail = (tail_temp + 1) % HID_WRITE_BUF_NUM;
  ret = buflen;
  if (hid_data->write_busy)
    {
      goto done;
    }

  hid_submit_data(hid_data);

done:
  KAL_SpinUnlockRestore((KalSpinLock *)&hid_data->hid_lock, flags);
  return ret;
}

int hid_fops_init(struct hid_dev_s *hid)
{
#ifdef CONFIG_DRIVERS_USB_HID_VFS_INTERFACE
  int ret = register_driver(USB_HID_DEV, &g_hid_fops, O_RDWR, hid);
  if (ret != OK)
    {
      usb_err("hid_dev register failed!\n");
      return ret;
    }
#else
  g_hid_dev = hid;
#endif

  g_hid_ops_cnt = 0;

  return 0;
}

int hid_fops_deinit(const struct hid_dev_s *hid)
{
  (void)hid;

#ifdef CONFIG_DRIVERS_USB_HID_VFS_INTERFACE
  int ret = unregister_driver(USB_HID_DEV);
  if (ret != OK)
    {
      usb_err("hid_dev unregister failed!\n");
      return ret;
    }
#else
  g_hid_dev = NULL;
#endif

  return 0;
}

#ifdef CONFIG_DRIVERS_USB_HID_FUNC_INTERFACE
static int check_and_add_hid_operations_count(void)
{
  uint32_t flags;

  flags = hid_acquire_lock();
  if (g_hid_ops_cnt == HID_FUNC_CNT_INVALID)
    {
      hid_release_lock(flags);
      usb_warn("hid device is not loaded!\n");
      return ERRCODE_USB_HID_DEVICE_NOT_LOADED;
    }

  g_hid_ops_cnt++;
  hid_release_lock(flags);

  return 0;
}

static void sub_hid_operations_count(void)
{
  uint32_t flags;

  flags = hid_acquire_lock();
  g_hid_ops_cnt--;
  hid_release_lock(flags);
}

#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT
#ifdef CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT_CALLBACK
bool fhid_register_output_callback(int32_t device_id, fhid_recv_cb_t cb)
{
  if (device_id >= HID_REPORT_MAP_NUM || g_hid_dev == NULL || cb == NULL)
    {
      return false;
    }
  struct hid_data_ctl *hid_data = &g_hid_dev->hid_data[device_id];
  hid_data->read_callback = cb;
  return true;
}

bool fhid_unregister_output_callback(int32_t device_id)
{
  if (device_id >= HID_REPORT_MAP_NUM || g_hid_dev == NULL)
    {
      return false;
    }
  struct hid_data_ctl *hid_data = &g_hid_dev->hid_data[device_id];
  hid_data->read_callback = NULL;
  return true;
}
#elif defined(CONFIG_DRIVERS_USB_HID_OUTPUT_REPORT_EVENT)

size_t fhid_recv_data(uint8_t report_index, char *buf, size_t buflen)
{
  size_t len;

  if (check_and_add_hid_operations_count() != 0)
    {
      return ERRCODE_USB_HID_DEVICE_NOT_LOADED;
    }

  len = hid_read_data(g_hid_dev, report_index, buf, buflen);

  sub_hid_operations_count();

  return len;
}
#endif
#endif

static int hid_prepare_get_report_data(uint8_t *data, uint16_t len)
{
  (void)memset_s(g_get_report_data, sizeof(g_get_report_data), 0, sizeof(g_get_report_data));
  if (memcpy_s(g_get_report_data, sizeof(g_get_report_data), data, len) != EOK) {
    return -1;
  }
  return len;
}
size_t fhid_send_data(uint8_t report_index, const char *buf, size_t buflen)
{
  size_t len;

  if (check_and_add_hid_operations_count() != 0)
    {
      return ERRCODE_USB_HID_DEVICE_NOT_LOADED;
    }
  hid_prepare_get_report_data((uint8_t *)buf, buflen);

  len = hid_write_data(g_hid_dev, report_index, buf, buflen);

  sub_hid_operations_count();

  return len;
}
#endif

size_t fhid_get_protocol(uint8_t device_id)
{
  if (device_id >= HID_REPORT_MAP_NUM || g_hid_dev == NULL)
    {
      return ERRCODE_INVALID_PARAM;
    }
  return g_hid_dev->hid_data[device_id].protocol;
}

void fhid_set_protocol(uint8_t device_id, uint32_t val)
{
  if (device_id >= HID_REPORT_MAP_NUM || g_hid_dev == NULL)
    {
      return;
    }
  g_hid_dev->hid_data[device_id].protocol = val;
}