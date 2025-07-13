/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2022. All rights reserved.
 * Description: LiteOS USB Driver Virtual Serial Port
 * Author: Huawei LiteOS Team
 * Create: 2016-07-06
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

#include "gadget/f_acm.h"

static volatile uint32_t g_usb_serial_mask;

static int usbclass_acm_bind(struct usbdevclass_driver_s *driver,
                             struct usbdev_s *dev);
static int usbclass_acm_unbind(struct usbdevclass_driver_s *driver,
                               struct usbdev_s *dev);
static int usbclass_acm_setup(struct usbdevclass_driver_s *driver,
                              struct usbdev_s *dev,
                              const struct usb_ctrlreq_s *ctrl, uint8_t *dataout,
                              size_t outlen);
static void usbclass_acm_disconnect(struct usbdevclass_driver_s *driver,
                                    struct usbdev_s *dev);

static void usbdev_acm_mkdevdesc(uint8_t *buf);
static int usbdev_acm_mkstrdesc(uint8_t id, struct usb_strdesc_s *buf);
static int16_t usbdev_acm_mkcfgdesc(uint8_t *buf, struct usbdev_devinfo_s *devinfo, uint8_t speed, uint8_t type);

static void acm_ep_req_deinit(struct usbdevclass_driver_s *driver,
                              struct usbdev_s *dev, int index);

static const struct usbdevclass_driverops_s g_acm_driverops =
{
  usbclass_acm_bind,       /* bind */
  usbclass_acm_unbind,     /* unbind */
  usbclass_acm_setup,      /* setup */
  usbclass_acm_disconnect, /* disconnect */
  NULL,                    /* suspend */
  NULL,                    /* resume */
};

#ifdef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
static const struct file_operations_vfs g_vfs_ops =
{
  .open   = usbclass_acm_open,
  .close  = NULL,
  .read   = usbclass_acm_read,
  .write  = usbclass_acm_write,
  .seek   = NULL,
  .ioctl  = usbclass_acm_ioctl,
  .poll   = NULL,
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
  .unlink = NULL,
#endif
};
#endif

static struct usb_devdesc_s g_acm_device_desc =
{
  .len          = sizeof(struct usb_devdesc_s),
  .type         = USB_DESC_TYPE_DEVICE,
  HSETW(.usb,   USB_VERSION_BCD), /* Usb version */
#ifdef CONFIG_DRIVERS_USB_SERIAL_GADGET_DUAL
  .classid      = USB_CLASS_MISC, /* 0xEF */
  .subclass     = 2,
  .protocol     = 1,
#else
  .classid      = USB_CLASS_CDC, /* 0x02 */
  .subclass     = 0,
  .protocol     = 0,
#endif
  .mxpacketsize = USB_EP_MPS,
  HSETW(.vendor,  0),             /* Vendor id */
  HSETW(.product, 0),             /* Product id */
  HSETW(.device, DEVICE_VERSION), /* Device version */
  .imfgr        = 1,
  .iproduct     = 2,
  .serno        = 3,
  .nconfigs     = 1,
};

struct acm_hs_function_descriptor_multi g_acm_hs_func_desc =
{
  .func_descriptor[0] = {
  {
    .len       = sizeof(struct usb_iaddesc_s),
    .type      = USB_DESC_TYPE_INTERFACEASSOCIATION,
    .firstif   = 0,
    .nifs      = 2,
    .classid   = 2, /* UFCLASS_CDC */
    .subclass  = 2,
    .protocol  = 1,

    /* Dynamic configuration when composite driver. 0x0c */

    .ifunction = 7,
  },
  {
    .len      = sizeof(struct usb_ifdesc_s),
    .type     = USB_DESC_TYPE_INTERFACE,
    .ifno     = 0,
    .alt      = 0,
    .neps     = 0x1,
    .classid  = USB_CLASS_CDC,
    .subclass = 0x2,
    .protocol = 0x1,

    /* Dynamic configuration when composite driver. 0x0a */

    .iif      = 5
  },
  {
    .bFunctionLength    = sizeof(struct cdc_header_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_HEADER_FUNC,
    .bcdCDC             = 0x0110
  },
  {
    .bFunctionLength    = sizeof(struct cdc_cm_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_CM_FUNC, /* Call Management */
    .bmCapabilities     = 0x0,
  },
  {
    .bFunctionLength    = sizeof(struct cdc_acm_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_ACM_FUNC, /* Abstract Control Model */
    .bmCapabilities     = CDC_ACM_REQ_LINE
  },
  {
    .bFunctionLength    = sizeof(struct cdc_union_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_UNION_FUNC,
  },
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_IN,
    .attr     = USB_EP_ATTR_XFER_INT,
    HSETW(.mxpacketsize, 0x000a),
    .interval = 9
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  },
#endif
  {
    .len      = sizeof(struct usb_ifdesc_s),
    .type     = USB_DESC_TYPE_INTERFACE,
    .ifno     = 1,
    .alt      = 0,
    .neps     = 0x2,
    .classid  = USB_CLASS_CDC_DATA,
    .subclass = 0x0,
    .protocol = 0x0,

    /* Dynamic configuration when composite driver. 0x0b */

    .iif      = 6
  },
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_IN,
    .attr     = USB_EP_ATTR_XFER_BULK,
    HSETW(.mxpacketsize, MAX_PACKET_SIZE),
    .interval = 0
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  },
#endif
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_OUT,
    .attr     = USB_EP_ATTR_XFER_BULK,
    HSETW(.mxpacketsize, MAX_PACKET_SIZE),
    .interval = 0
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  }
#endif
},
#ifdef CONFIG_DRIVERS_USB_SERIAL_GADGET_DUAL
.func_descriptor[1] = {
  {
    .len       = sizeof(struct usb_iaddesc_s),
    .type      = USB_DESC_TYPE_INTERFACEASSOCIATION,
    .firstif   = 2,
    .nifs      = 2,
    .classid   = 2, /* UFCLASS_CDC */
    .subclass  = 2,
    .protocol  = 1,
    .ifunction = 7  /* Index of string descriptor */
  },
  {
    .len      = sizeof(struct usb_ifdesc_s),
    .type     = USB_DESC_TYPE_INTERFACE,
    .ifno     = 2,
    .alt      = 0,
    .neps     = 0x1,
    .classid  = USB_CLASS_CDC,
    .subclass = 2,
    .protocol = 1,

    /* Dynamic configuration when composite driver. 0x0a */

    .iif      = 5
  },
  {
    .bFunctionLength    = sizeof(struct cdc_header_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_HEADER_FUNC,
    .bcdCDC             = 0x0110,
  },
  {
    .bFunctionLength    = sizeof(struct cdc_cm_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_CM_FUNC, /* Call Management */
    .bmCapabilities     = 0x0,
  },
  {
    .bFunctionLength    = sizeof(struct cdc_acm_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_ACM_FUNC, /* Abstract Control Model */
    .bmCapabilities     = CDC_ACM_REQ_LINE
  },
  {
    .bFunctionLength    = sizeof(struct cdc_union_func_desc),
    .bDescriptorType    = USB_DESC_TYPE_CSINTERFACE,
    .bDescriptorSubtype = USB_CDC_UNION_FUNC,
  },
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_IN,
    .attr     = USB_EP_ATTR_XFER_INT,
    HSETW(.mxpacketsize, 0x000a),
    .interval = 9
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  },
#endif
  {
    .len      = sizeof(struct usb_ifdesc_s),
    .type     = USB_DESC_TYPE_INTERFACE,
    .ifno     = 3,
    .alt      = 0,
    .neps     = 0x2, /* bNumEndpoints: Two endpoints used */
    .classid  = USB_CLASS_CDC_DATA,
    .subclass = 0x0, /* bInterfaceSubClass: Data Class SubClass code */
    .protocol = 0x0,

    /* Dynamic configuration when composite driver. 0x0b */

    .iif      = 6
  },
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_IN,
    .attr     = USB_EP_ATTR_XFER_BULK,
    HSETW(.mxpacketsize, MAX_PACKET_SIZE),
    .interval = 0
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  },
#endif
  {
    .len      = sizeof(struct usb_epdesc_s),
    .type     = USB_DESC_TYPE_ENDPOINT,
    .addr     = USB_DIR_OUT,
    .attr     = USB_EP_ATTR_XFER_BULK,
    HSETW(.mxpacketsize, MAX_PACKET_SIZE),
    .interval = 0
  },
#ifdef DWC3_USB_SERIAL
  {
    .len      = 6,
    .type     = 0x30,
    .maxburst = 0,
    .attr     = 0,
    .interval = {0}
  }
#endif
}
#endif
};

/* Config contain func */

static const struct usb_cfgdesc_s g_acm_hs_config_desc =
{
  .len         = USB_SIZEOF_CFGDESC,
  .type        = USB_DESC_TYPE_CONFIG,
  HSETW(.totallen, USB_SIZEOF_CFGDESC + sizeof(g_acm_hs_func_desc)),
  .ninterfaces = ACM_IF_NUM_PER_PORT * NUM_SERIAL,
  .cfgvalue    = 2,
  .icfg        = 4,
  .attr        = USB_CONFIG_ATTR_SELFPOWER | USB_CONFIG_ATTR_ONE, /* 0xc0 */
  .mxpower     = 1 /* Max power */
};

#define DT_STRING_ID_LEN 4
static const char g_dt_string_id[DT_STRING_ID_LEN] =
{
  DT_STRING_ID_LEN,
  USB_DESC_TYPE_STRING,
  0x09, 0x04
};

#define DT_STRING_4_LEN 30
static const char g_dt_string_4[DT_STRING_4_LEN] =
{
  DT_STRING_4_LEN,
  USB_DESC_TYPE_STRING,
  0x43, 0, 0x44, 0, 0x43, 0, 0x20, 0, 0x41, 0, 0x43, 0, 0x4d, 0,
  0x20, 0, 0x63, 0, 0x6f, 0, 0x6e, 0, 0x66, 0, 0x69, 0, 0x67, 0
};

#define DT_STRING_5_LEN 66
static const char g_dt_string_5[DT_STRING_5_LEN] =
{
  DT_STRING_5_LEN,
  USB_DESC_TYPE_STRING,
  0x43, 0, 0x44, 0, 0x43, 0, 0x20, 0, 0x41, 0, 0x62, 0, 0x73, 0, 0x74, 0, 0x72, 0, 0x61, 0,
  0x63, 0, 0x74, 0, 0x20, 0, 0x43, 0, 0x6f, 0, 0x6e, 0, 0x74, 0, 0x72, 0, 0x6f, 0, 0x6c, 0,
  0x20, 0, 0x4d, 0, 0x6f, 0, 0x64, 0, 0x65, 0, 0x6c, 0, 0x20, 0, 0x28, 0, 0x41, 0, 0x43, 0,
  0x4d, 0, 0x29, 0
};

#define DT_STRING_6_LEN 26
static const char g_dt_string_6[DT_STRING_6_LEN] =
{
  DT_STRING_6_LEN,
  USB_DESC_TYPE_STRING,
  0x43, 0, 0x44, 0, 0x43, 0, 0x20, 0, 0x41, 0,
  0x43, 0, 0x4d, 0, 0x20, 0, 0x44, 0, 0x61, 0, 0x74, 0, 0x61, 0
};

#define DT_STRING_7_LEN 22
static const char g_dt_string_7[DT_STRING_7_LEN] =
{
  DT_STRING_7_LEN,
  USB_DESC_TYPE_STRING,
  0x43, 0, 0x44, 0, 0x43, 0, 0x20, 0, 0x53, 0, 0x65, 0,
  0x72, 0, 0x69, 0, 0x61, 0, 0x6c, 0
};

#define FSERIAL_DSTR_IDX 9
static struct usbd_string g_acm_device_strings[FSERIAL_DSTR_IDX] =
{
  { 0, g_dt_string_id },
  { 1, NULL },
  { 2, NULL },
  { 3, NULL },
  { 4, g_dt_string_4 },
  { 5, g_dt_string_5 },
  { 6, g_dt_string_6 },
  { 7, g_dt_string_7 },
  USBD_DEVICE_STRINGS_END
};

#ifndef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
struct acm_driver_s *g_acm_driver = NULL;

struct acm_driver_s *acm_get_acm_driver(void)
{
  return g_acm_driver;
}
#endif

void acm_get_device_dec_info(struct usbd_string **device_strings,
                             struct usb_devdesc_s **device_desc)
{
  *device_strings = &g_acm_device_strings[1];
  *device_desc    = &g_acm_device_desc;
}

uint32_t userial_mask_get(void)
{
  return g_usb_serial_mask;
}

void userial_mask_set(uint32_t val)
{
  g_usb_serial_mask = val;
}

static void acm_reset_eps_config(struct acm_data_ctrl *datactrl)
{
  if (datactrl == NULL)
    {
      usb_err("Disable ep failed!\n");
      return;
    }

  if (datactrl->epintin_enabled == true)
    {
      EP_DISABLE(datactrl->epintin);
      datactrl->epintin_enabled = false;
    }

  if (datactrl->epbulkin_enabled == true)
    {
      EP_DISABLE(datactrl->epbulkin);
      datactrl->epbulkin_enabled = false;
    }

  if (datactrl->epbulkout_enabled == true)
    {
      EP_DISABLE(datactrl->epbulkout);
      datactrl->epbulkout_enabled = false;
    }
}

static int acm_set_ep_config(struct usbdev_ep_s *ep,
                             enum acm_ep_index epid,
                             const struct usb_epdesc_s *ep_desc,
                             struct usbdev_s *dev,
                             bool last)
{
  if (epid != ACM_INTERRUPT_EP_IDX)
    {
      usbd_configep_byspeed(dev, (struct usb_epdesc_s *)ep_desc);
    }
  return EP_CONFIGURE(ep, ep_desc, last);
}

static int acm_set_serial_config(struct acm_dev_s *priv,
                                 struct usbdev_s *dev, int index)
{
  struct acm_data_ctrl *datactrl;
  struct acm_rdreq_s *rdcontainer;
  struct acm_hs_function_descriptor *func_desc;
  int ret;
  uint32_t flags;

  if (priv == NULL)
    {
      return -EINVAL;
    }

  datactrl = &priv->datactrl[index];
  rdcontainer = &datactrl->rdreqs;

  if (datactrl->epintin_enabled == true)
    {
      EP_DISABLE(datactrl->epintin);
      datactrl->epintin_enabled = false;
    }

  func_desc = &g_acm_hs_func_desc.func_descriptor[index];

  ret = acm_set_ep_config(datactrl->epintin, ACM_INTERRUPT_EP_IDX,
                          (const  struct usb_epdesc_s *)&func_desc->nepd,
                          dev, false);
  if (ret < 0)
    {
      usb_err("config interrupt ep failed!\n");
      goto errout;
    }

  datactrl->epintin_enabled = true;

  if (datactrl->epbulkin_enabled == true)
    {
      EP_DISABLE(datactrl->epbulkin);
      datactrl->epbulkin_enabled = false;
    }

  ret = acm_set_ep_config(datactrl->epbulkin, ACM_BULKIN_EP_IDX,
                          (const struct usb_epdesc_s *)&func_desc->iepd,
                          dev, false);
  if (ret < 0)
    {
      usb_err("config bulk in ep failed!\n");
      goto errout;
    }

  datactrl->epbulkin_enabled = true;

  if (datactrl->epbulkout_enabled == true)
    {
      EP_DISABLE(datactrl->epbulkout);
      datactrl->epbulkout_enabled = false;
    }

  ret = acm_set_ep_config(datactrl->epbulkout, ACM_BULKOUT_EP_IDX,
                          (const struct usb_epdesc_s *)&func_desc->oepd,
                          dev, false);
  if (ret < 0)
    {
      usb_err("config bulk out ep failed!\n");
      goto errout;
    }

  datactrl->epbulkout_enabled = true;

  ret = acm_send_out_req(datactrl, rdcontainer);
  if (ret != OK)
    {
      goto errout;
    }

  spin_lock_irqsave(&datactrl->acm_lock, flags);
  ret = acm_serial_connect(priv, index);
  if (ret < 0)
    {
      usb_err("acm device connect failed!\r\n");
      spin_unlock_irqrestore(&datactrl->acm_lock, flags);
      goto errout;
    }

  LOS_AtomicSet(&datactrl->shutdown, 0);
  datactrl->bdisconnect = false;

  userial_mask_set(1);
  spin_unlock_irqrestore(&datactrl->acm_lock, flags);

  return OK;

errout:
  acm_reset_eps_config(datactrl);
  return ret;
}

static struct usbdev_req_s *acm_alloc_req(struct usbdev_ep_s *ep, uint16_t len)
{
  struct usbdev_req_s *req;

  if (ep == NULL)
    {
      return NULL;
    }

  req = EP_ALLOCREQ(ep);
  if (req == NULL)
    {
      return NULL;
    }

  req->len = len;
  req->buf = EP_ALLOCBUFFER(ep, len);
  if (req->buf == NULL)
    {
      EP_FREEREQ(ep, req);
      req = NULL;
    }

  return req;
}

static void acm_free_req(struct usbdev_ep_s *ep, struct usbdev_req_s *req)
{
  if (ep == NULL || req == NULL)
    {
      return;
    }

  if (req->buf != NULL)
    {
      EP_FREEBUFFER(ep, req->buf);
    }

  EP_FREEREQ(ep, req);
}

static int acm_init_eps(struct usbdevclass_driver_s *driver,
                        struct usbdev_s *dev, int index)
{
  struct acm_dev_node *cnode;
  struct acm_data_ctrl *datactrl;
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct composite_dev_s *cdev;
  struct composite_devobj_s *devobj;
  struct usbdev_devinfo_s *devinfo;
  struct acm_hs_function_descriptor *func_desc;
  struct acm_wrreq_s *wrcontainer;
#ifdef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  char dev_name[ACM_DEVNAME_SIZE];
#endif
  int ret;

  priv->usbdev = dev;
  datactrl = &priv->datactrl[index];

  cnode = (struct acm_dev_node *)malloc(sizeof(struct acm_dev_node));
  if (cnode == NULL)
    {
      usb_err("malloc failed\n");
      ret = -ENOMEM;
      goto errout;
    }

  cnode->dev = priv;
  cnode->index = index;

  cdev = dev->ep0->priv;
  datactrl->bdisconnect = true;
  LOS_AtomicSet(&datactrl->shutdown, 0);
  (void)LOS_EventInit(&datactrl->read_event);
  spin_lock_init(&datactrl->acm_lock);

#ifdef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  if (snprintf_s(dev_name, ACM_DEVNAME_SIZE, ACM_DEVNAME_SIZE - 1, ACM_DEVNAME_FORMAT, index) < 0)
    {
      usb_err("snprintf_s failed!\n");
      ret = -EINVAL;
      goto free_out;
    }

  ret = register_driver(dev_name, &g_vfs_ops, ACM_DRIVE_MODE, cnode);
  if (ret != 0)
    {
      usb_err("chardev register failed! ret = %d", ret);
      goto free_out;
    }
#endif

  devobj = usbclass_devobj_get(cdev, DEV_SERIAL);
  if (devobj == NULL)
    {
      ret = -EINVAL;
      goto free_out;
    }

  devinfo = &devobj->compdesc.devinfo;

  func_desc = &g_acm_hs_func_desc.func_descriptor[index];

  datactrl->epintin = DEV_ALLOCEP(dev, func_desc->nepd.addr, &func_desc->nepd);
  if (datactrl->epintin == NULL)
    {
      usb_err("alloc epintin fail!");
      ret = -ENODEV;
      goto free_out;
    }

  ACM_SET_INTERRUPT_EP_NUM(devinfo, index) = datactrl->epintin->eplog;
  datactrl->epintin->priv = cnode;

  datactrl->epbulkin = DEV_ALLOCEP(dev, func_desc->iepd.addr, &func_desc->iepd);
  if (datactrl->epbulkin == NULL)
    {
      usb_err("alloc epbulkin fail!\n");
      ret = -ENODEV;
      goto free_out;
    }

  ACM_SET_BULKIN_EP_NUM(devinfo, index) = datactrl->epbulkin->eplog;
  datactrl->epbulkin->priv = cnode;

  datactrl->epbulkout = DEV_ALLOCEP(dev, func_desc->oepd.addr, &func_desc->oepd);
  if (datactrl->epbulkout == NULL)
    {
      usb_err("alloc epbulkout fail!");
      ret = -ENODEV;
      goto free_out;
    }

  ACM_SET_BULKOUT_EP_NUM(devinfo, index) = datactrl->epbulkout->eplog;
  datactrl->epbulkout->priv = cnode;

  wrcontainer = &datactrl->wrreqs;

  wrcontainer->data_buffer = malloc(RING_BUFFER_SIZE);
  if (wrcontainer->data_buffer == NULL)
    {
      ret = -ENOMEM;
      goto free_out;
    }

  (void)memset_s(wrcontainer->data_buffer, RING_BUFFER_SIZE, 0, RING_BUFFER_SIZE);
  wrcontainer->receive_buffer = wrcontainer->data_buffer;
  wrcontainer->send_buffer    = wrcontainer->data_buffer;

  return OK;

free_out:
  free(cnode);

errout:
  return ret;
}

static int acm_reqs_init(struct usbdevclass_driver_s *driver, int index)
{
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct acm_data_ctrl *datactrl = &priv->datactrl[index];
  struct usbdev_ep_s *epintin;
  struct usbdev_ep_s *epbulkin;
  struct usbdev_ep_s *epbulkout;
  struct acm_wrreq_s *wrcontainer;
  struct acm_rdreq_s *rdcontainer;
  uint16_t reqlen;

  /* initialize interrupt req */

  epintin = datactrl->epintin;
  epintin->handle_req = acm_alloc_req(epintin,
                                      sizeof(struct cdc_notification_element) + sizeof(uint16_t));
  if (epintin->handle_req == NULL)
    {
      return -1;
    }

  epintin->handle_req->callback = acm_ep0incomplete_notify_callback;

  /* initialize read req */

  reqlen = ACM_OUT_REQ_LEN;

  epbulkout = datactrl->epbulkout;
  epbulkout->maxpacket = reqlen;

  rdcontainer = &datactrl->rdreqs;

  rdcontainer->req = acm_alloc_req(epbulkout, reqlen);
  if (rdcontainer->req == NULL)
    {
      usb_err("alloc rd_req failed!\n");
      return -1;
    }

  rdcontainer->buflen        = reqlen;
  rdcontainer->req->priv     = rdcontainer;
  rdcontainer->req->callback = acm_rdcomplete_process_req;
  epbulkout->handle_req      = rdcontainer->req;

  /* initialize write req */

  reqlen = ACM_IN_REQ_LEN;

  epbulkin = datactrl->epbulkin;
  wrcontainer = &datactrl->wrreqs;
  wrcontainer->req = acm_alloc_req(epbulkin, reqlen);
  if (wrcontainer->req == NULL)
    {
      usb_err("alloc wr_req failed!\n");
      return -1;
    }

  wrcontainer->req->priv     = wrcontainer;
  wrcontainer->req->callback = acm_wrcomplete_check_send_result;
  epbulkin->handle_req = wrcontainer->req;

  datactrl->acm_flag = 0;

  return OK;
}

static int acm_ep_req_init(struct usbdevclass_driver_s *driver,
                           struct usbdev_s *dev, int index)
{
  int ret;

  ret = acm_init_eps(driver, dev, index);
  if (ret != OK)
    {
      return ret;
    }

  ret = acm_reqs_init(driver, index);
  if (ret != OK)
    {
      return ret;
    }

  return OK;
}

static int usbclass_acm_bind(struct usbdevclass_driver_s *driver,
                             struct usbdev_s *dev)
{
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct composite_devobj_s *devobj;
  struct usbdev_devinfo_s *devinfo;
  struct composite_dev_s *cdev;
  int ret;

  cdev = dev->ep0->priv;

  priv->ctrlreq = cdev->ctrlreq;
  if (priv->ctrlreq == NULL)
    {
      ret = -EINVAL;
      goto errout;
    }

  devobj = usbclass_devobj_get(cdev, DEV_SERIAL);
  if (devobj == NULL)
    {
      ret = -EINVAL;
      goto errout;
    }

  devinfo = &devobj->compdesc.devinfo;

  priv->ctrl_id = (uint8_t)devinfo->ifnobase;
  priv->data_id = (uint8_t)devinfo->ifnobase + 1;

  priv->line_state.dwDTERate   = ACM_DATA_SPEED;
  priv->line_state.bCharFormat = CDC_STOP_BIT_1;
  priv->line_state.bParityType = CDC_PARITY_NONE;
  priv->line_state.bDataBits   = ACM_DATA_BIT;

  if (LOS_EventInit(&priv->unbind_event) != LOS_OK)
    {
      ret = -EINVAL;
      goto errout;
    }

  spin_lock_init(&priv->state_lock);

  for (int index = 0; index < NUM_SERIAL; index++)
    {
      ret = acm_ep_req_init(driver, dev, index);
      if (ret != OK)
        {
          goto errout;
        }
    }

#ifndef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  g_acm_driver = (struct acm_driver_s *)driver;
#endif

  return OK;

errout:
  (void)usbclass_acm_unbind(driver, dev);
  return ret;
}

static void acm_end_read_waiting(struct usbdevclass_driver_s *driver)
{
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct acm_data_ctrl *datactrl;

  for (int index = 0; index < NUM_SERIAL; index++)
    {
      datactrl = &priv->datactrl[index];
      if (!LOS_ListEmpty(&datactrl->read_event.stEventList))
        {
          (void)LOS_EventWrite(&datactrl->read_event, USB_SERIAL_READ_EVENT);
        }
    }
}

static int usbclass_acm_unbind(struct usbdevclass_driver_s *driver,
                               struct usbdev_s *dev)
{
  struct acm_dev_s *priv;
  uint32_t lock_flag;
  uint8_t state_flag = 0;

  if (driver == NULL || dev == NULL)
    {
      usb_err("no device!\n");
      return -EINVAL;
    }

  priv = &((struct acm_driver_s *)driver)->dev;
  spin_lock_irqsave(&priv->state_lock, lock_flag);
  for (int i = 0; i < NUM_SERIAL; i++)
    {
      priv->datactrl[i].acm_flag |= ACM_UNBIND_STATE;
      state_flag |= priv->datactrl[i].acm_flag;
    }
  spin_unlock_irqrestore(&priv->state_lock, lock_flag);

#ifndef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  g_acm_driver = NULL;
#endif

  if (state_flag != ACM_UNBIND_STATE)
    {
      if (state_flag & ACM_READ_BUSY_STATE)
        {
          acm_end_read_waiting(driver);
        }
      (void)LOS_EventRead(&priv->unbind_event, ACM_UNBIND_EVENT,
                          LOS_WAITMODE_OR | LOS_WAITMODE_CLR, CONFIG_BASE_CORE_TICK_PER_SECOND);
    }

  (void)LOS_EventDestroy(&priv->unbind_event);

  for (int i = 0; i < NUM_SERIAL; i++)
    {
      acm_ep_req_deinit(driver, dev, i);
    }
  return OK;
}

static int acm_req_set_configuration(struct acm_dev_s *priv, struct usbdev_s *dev)
{
  int ret;

  for (int i = 0; i < NUM_SERIAL; i++)
    {
      ret = acm_set_serial_config(priv, dev, i);
    }

  return ret;
}

static void usbdev_acm_uninitialize(struct usbdevclass_driver_s *classdev)
{
  struct acm_driver_s *drvr = (struct acm_driver_s *)classdev;
  struct usbd_string *device_strings;
  struct usb_devdesc_s *device_desc;

  acm_get_device_dec_info(&device_strings, &device_desc);
  dev_str_desc_free(device_strings);
  (void)device_desc;

  if (drvr == NULL)
    {
      usb_err("Invalid pointer!\n");
      return;
    }

  free(drvr);
}

static void acm_free_reqs(struct usbdevclass_driver_s *driver,
                          struct usbdev_s *dev, int index)
{
  struct acm_dev_s *priv;
  struct acm_data_ctrl *datactrl;
  struct acm_wrreq_s *wrcontainer;
  struct acm_rdreq_s *rdcontainer;

  priv = &((struct acm_driver_s *)driver)->dev;

  /* Free the pre-allocated interrupt Request */

  datactrl = &priv->datactrl[index];
  if (datactrl->epintin != NULL && datactrl->epintin->handle_req != NULL)
    {
      if (datactrl->epintin->priv != NULL)
        {
          free(datactrl->epintin->priv);
          datactrl->epintin->priv = NULL;
        }

      acm_free_req(datactrl->epintin, datactrl->epintin->handle_req);
      datactrl->epintin->handle_req = NULL;
    }

  /* Free the interrupt endpoint */

  if (datactrl->epintin != NULL)
    {
      DEV_FREEEP(dev, datactrl->epintin);
      datactrl->epintin = NULL;
    }

  /* Free pre-allocated read request */

  rdcontainer = &datactrl->rdreqs;
  if (rdcontainer->req != NULL)
    {
      acm_free_req(datactrl->epbulkout, rdcontainer->req);
      rdcontainer->req = NULL;
    }

  /* Free the bulk_out endpoint */

  if (datactrl->epbulkout != NULL)
    {
      DEV_FREEEP(dev, datactrl->epbulkout);
      datactrl->epbulkout = NULL;
    }

  /* Free pre-allocated write request */

  wrcontainer = &datactrl->wrreqs;
  if (wrcontainer->req != NULL)
    {
      acm_free_req(datactrl->epbulkin, wrcontainer->req);
      wrcontainer->req = NULL;
    }

  /* Free the bulk_in endpoint */

  if (datactrl->epbulkin != NULL)
    {
      DEV_FREEEP(dev, datactrl->epbulkin);
      datactrl->epbulkin = NULL;
    }
}

static int acm_copy_line_state_to_buf(uint8_t *buf, struct acm_dev_s *priv)
{
  int ret;
  ret = memcpy_s(buf, sizeof(struct cdc_line_coding_state), (uint8_t *)&priv->line_state,
                 SIZEOF_ACM_LINE_STATE);
  if (ret != EOK)
    {
      usb_err("memcpy_s fail, ret:%d\n", ret);
      return ret;
    }
  return SIZEOF_ACM_LINE_STATE;
}

static int usbdev_acm_classobject(int minor, struct usbdev_devinfo_s *devinfo,
                                  struct usbdevclass_driver_s **classdev)
{
  struct acm_dev_s *priv;
  struct acm_driver_s *acm_drvr;

  acm_drvr = (struct acm_driver_s *)malloc(sizeof(struct acm_driver_s));
  if (acm_drvr == NULL)
    {
      usb_err("malloc failed!\n");
      return -ENOMEM;
    }

  priv = &acm_drvr->dev;

  (void)memset(acm_drvr, 0, sizeof(struct acm_driver_s));
  (void)memcpy(&priv->devinfo, devinfo, sizeof(struct usbdev_devinfo_s));

  acm_drvr->drvr.speed = USB_SPEED_HIGH;
  acm_drvr->drvr.ops   = &g_acm_driverops;
  *classdev            = &acm_drvr->drvr;

  return 0;
}

static void acm_resource_free(struct usbdevclass_driver_s *driver, int index)
{
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct acm_data_ctrl *datactrl = &priv->datactrl[index];

  struct acm_wrreq_s *wrcontainer;
#ifdef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  char dev_name[ACM_DEVNAME_SIZE];
#endif

  /* Destroy read event */

  (void)LOS_EventDestroy(&datactrl->read_event);

  /* Free ringbuffer */

  wrcontainer = &datactrl->wrreqs;
  if (wrcontainer->data_buffer != NULL)
    {
      free(wrcontainer->data_buffer);
      wrcontainer->data_buffer = NULL;
    }

#ifdef CONFIG_DRIVERS_USB_SERIAL_VFS_INTERFACE
  /* Unregister driver */

  if (snprintf_s(dev_name, ACM_DEVNAME_SIZE, ACM_DEVNAME_SIZE - 1, ACM_DEVNAME_FORMAT, index) < 0)
    {
      usb_err("snprintf_s failed!\n");
      return;
    }

  if (unregister_driver(dev_name) != OK)
    {
      usb_err("chardev unregister failed!\n");
    }
#endif
}

static void usbclass_acm_disconnect(struct usbdevclass_driver_s *driver,
                                    struct usbdev_s *dev)
{
  struct acm_dev_s *priv = &((struct acm_driver_s *)driver)->dev;
  struct acm_data_ctrl *datactrl;

  for (int i = 0; i < NUM_SERIAL; i++)
    {
      datactrl = &priv->datactrl[i];
      if (datactrl->bdisconnect == false)
        {
          (void)acm_serial_disconnect(priv, i);
          acm_reset_eps_config(datactrl);
          datactrl->bdisconnect = true;
          LOS_AtomicSet(&datactrl->shutdown, 1);
        }
    }
}

static void acm_ep_req_deinit(struct usbdevclass_driver_s *driver,
                              struct usbdev_s *dev, int index)
{
  /* Close device */

  usbclass_acm_disconnect(driver, dev);

  acm_free_reqs(driver, dev, index);

  acm_resource_free(driver, index);
}

static void acm_set_line_complete(struct usbdev_ep_s *ep,
                                  struct usbdev_req_s *req)
{
  struct acm_dev_s *priv;
  errno_t ret;

  (void)ep;
  priv = req->priv;
  if (req->result != 0)
    {
      return;
    }

  userial_mask_set(1);
  ret = memcpy_s((uint8_t *)&priv->line_state, sizeof(struct cdc_line_coding_state),
                 req->buf, SIZEOF_ACM_LINE_STATE);
  if (ret != EOK)
    {
      usb_err("memcpy_s fail!, ret:%d\n", ret);
      return;
    }
}

static int usbclass_acm_setup(struct usbdevclass_driver_s *driver,
                              struct usbdev_s *dev,
                              const struct usb_ctrlreq_s *ctrl, uint8_t *dataout,
                              size_t outlen)
{
  struct acm_dev_s *priv;
  struct usbdev_req_s *ctrlreq;
  uint16_t index;
  uint16_t len;
  int ret = -EOPNOTSUPP;

  if (driver == NULL || dev == NULL || ctrl == NULL)
    {
      return -EINVAL;
    }

  priv = &((struct acm_driver_s *)driver)->dev;

  ctrlreq = priv->ctrlreq;

  index = USB_GETW(ctrl->index);
  len   = USB_GETW(ctrl->len);

  /* Standard protocol requests are placed at the composite layer,
   * and specific protocol requests are placed at the corresponding protocol.
   */

  if (ctrl->req == USB_REQ_SETCONFIGURATION && ctrl->type == 0)
    {
      return acm_req_set_configuration(priv, dev);
    }

  if ((ctrl->type & USB_REQ_TYPE_MASK) != USB_REQ_TYPE_CLASS)
    {
      goto failed_out;
    }

  if ((ctrl->type & ACM_REQ_TYPE_INTERFACE) != (USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE))
    {
      goto failed_out;
    }

#ifndef CONFIG_DRIVERS_USB_SERIAL_GADGET_DUAL
  if (index != priv->devinfo.ifnobase)
    {
      goto failed_out;
    }
#endif

  switch (ctrl->req)
    {
    case ACM_GET_LINE_STATE:
      {
        if ((ctrl->type & USB_DIR_MASK) != USB_DIR_IN)
          {
            goto failed_out;
          }

        ret = acm_copy_line_state_to_buf(ctrlreq->buf, priv);
        if (ret != SIZEOF_ACM_LINE_STATE)
          {
            return ret;
          }
      }
      break;

    case ACM_SET_LINE_STATE:
      {
        if (len != SIZEOF_ACM_LINE_STATE)
          {
            goto failed_out;
          }

        if ((ctrl->type & USB_DIR_MASK) != USB_DIR_OUT)
          {
            goto failed_out;
          }
        ctrlreq->priv     = priv;
        ctrlreq->callback = acm_set_line_complete;
        ret               = len;
      }
      break;

    case ACM_SET_CTRL_LINE_STATE:
      {
        if ((ctrl->type & USB_DIR_MASK) != USB_DIR_OUT)
          {
            goto failed_out;
          }

        /* Respond with a zero length packet */

        ret = 0;
      }
      break;

    default:
      {
        break;
      }
    }

  if (ret >= 0)
    {
      /* Configure the response */

      ctrlreq->len = (uint32_t)ret;
      ret          = EP_SUBMIT(dev->ep0, ctrlreq);
      if (ret != OK)
        {
          usb_err("acm response on err %d\n", ret);
          ctrlreq->result = OK;
        }
      return ret;
    }

failed_out:
  usb_err("type=%02x ctrlreq=%02x index=%04x len=%04x\n",
          ctrl->type, ctrl->req, index, len);
  return ret;
}

void usbdev_acm_initialize_dev(struct composite_devdesc_s *dev, int ifnobase, int minor)
{
  (void)memset(dev, 0, sizeof(struct composite_devdesc_s));

  /* Determine the necessary interfaces. */

  dev->mkdevdesc  = usbdev_acm_mkdevdesc;
  dev->mkconfdesc = usbdev_acm_mkcfgdesc;
  dev->mkstrdesc  = usbdev_acm_mkstrdesc;

  dev->classobject  = usbdev_acm_classobject;
  dev->uninitialize = usbdev_acm_uninitialize;

  /* Init ifnobase of usbdev_devinfo_s for judegment ctrl->index */

  dev->nconfigs = ACM_CONFIG_NUM; /* Number of configurations supported */
  dev->configid = ACM_CONFIG_ID;  /* The only supported configuration ID */
  dev->minor    = minor;          /* The minor interface number */

  dev->devinfo.strbase     = 0;                 /* Offset to String Numbers */
  dev->devinfo.ifnobase    = ifnobase;          /* Offset to Interface-IDs */
  dev->devinfo.ninterfaces = ACM_INTERFACE_NUM; /* Number of interfaces in the configuration */
  dev->devinfo.nstrings    = ACM_STR_ID_NUM;
  dev->devinfo.nendpoints  = ACM_TOTAL_EP_NUM;
}

/* Init composite public action */

int usbdev_acm_initialize(void *handle)
{
  struct composite_devdesc_s dev;
  int ret;

  usbdev_acm_initialize_dev(&dev, 0, DEV_SERIAL);
  ret = composite_initialize_softc(1, &dev, handle);
  if (ret)
    {
      return -1;
    }
  PRINTK(" ** Serial device initialized successfully! **\n ");

  return 0;
}

static void usbdev_acm_mkdevdesc(uint8_t *buf)
{
  errno_t ret;

  if (USB_GETW(g_acm_device_desc.vendor) == 0)
    {
      usb_err("VID is not set!\n");
      return;
    }

  ret = memcpy_s(buf, USB_COMP_EP0_BUFSIZ, &g_acm_device_desc, sizeof(g_acm_device_desc));
  if (ret != EOK)
    {
      usb_err("memcpy_s fail!, ret:%d\n", ret);
      return;
    }
}

static void acm_set_func_descriptor(struct acm_hs_function_descriptor *desc,
                                    uint8_t ifnobase, uint8_t index)
{
  uint8_t interface_count                = desc->ifcad.nifs * index;
  desc->ifcad.firstif                    = ifnobase + interface_count;
  desc->ifcd.ifno                        = ifnobase + interface_count;
  desc->cdc_union_desc.bMasterInterface  = ifnobase + interface_count;
  desc->ifdd.ifno                        = ifnobase + interface_count + 1;
  desc->cdc_union_desc.bSlaveInterface0  = ifnobase + interface_count + 1;
  desc->cdc_call_desc.bDataInterface     = ifnobase + interface_count + 1;
}

static int16_t usbdev_acm_mkcfgdesc(uint8_t *buf, struct usbdev_devinfo_s *devinfo, uint8_t speed, uint8_t type)
{
  int16_t total_len = 0;
  errno_t ret;

  for (uint8_t i = 0; i < NUM_SERIAL; i++)
    {
      acm_set_func_descriptor(&g_acm_hs_func_desc.func_descriptor[i], (uint8_t)devinfo->ifnobase, i);
    }

  /* Copy Serial device configure descriptor. */

  ret = memcpy_s(buf, USB_COMP_EP0_BUFSIZ, (const void *)&g_acm_hs_config_desc, USB_SIZEOF_CFGDESC);
  if (ret != EOK)
    {
      usb_err("memcpy_s fail, ret:%d\n", ret);
      return -1;
    }
  total_len += USB_SIZEOF_CFGDESC;

  /* Copy Serial device function descriptor. */

  buf += USB_SIZEOF_CFGDESC;
  ret = memcpy_s(buf, (USB_COMP_EP0_BUFSIZ - total_len),
                 (const void *)&g_acm_hs_func_desc, sizeof(g_acm_hs_func_desc));
  if (ret != EOK)
    {
      usb_err("memcpy_s fail, ret:%d\n", ret);
      return -1;
    }
  total_len += sizeof(g_acm_hs_func_desc);

  return total_len;
}

static int usbdev_acm_mkstrdesc(uint8_t id, struct usb_strdesc_s *buf)
{
  errno_t ret;
  const char *str;
  int i;

  for (i = 0; g_acm_device_strings[i].s != NULL; i++)
    {
      str = g_acm_device_strings[i].s;
      if (g_acm_device_strings[i].id == id)
        {
          ret = memcpy_s(buf, USB_COMP_EP0_BUFSIZ, (const void *)str, (uint32_t)str[0]);
          if (ret != EOK)
            {
              usb_err("memcpy_s failed, ret = %d\n", ret);
              return -1;
            }
          return str[0];
        }
    }

  usb_err("Can not find the id = %u of string\n", id);
  return -1;
}
