/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <linux/usb/ch9.h>



#if 1
 #define v4l2_fourcc(a, b, c, d) \
    ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

 #define VIDIOC_S_FMT _IOWR('V', 5, struct v4l2_format)
 #define VIDIOC_REQBUFS _IOWR('V', 8, struct v4l2_requestbuffers)
 #define VIDIOC_QUERYBUF _IOWR('V', 9, struct v4l2_buffer)
 #define VIDIOC_QUERYCAP _IOR('V', 0, struct v4l2_capability)
 #define VIDIOC_STREAMOFF _IOW('V', 19, int)
 #define VIDIOC_QBUF _IOWR('V', 15, struct v4l2_buffer)
 #define VIDIOC_S_CTRL _IOWR('V', 28, struct v4l2_control)
 #define VIDIOC_S_DV_TIMINGS _IOWR('V', 87, struct v4l2_dv_timings)
 #define VIDIOC_G_DV_TIMINGS _IOWR('V', 88, struct v4l2_dv_timings)
 #define VIDIOC_DQEVENT _IOR('V', 89, struct v4l2_event)
 #define VIDIOC_SUBSCRIBE_EVENT _IOW('V', 90, struct v4l2_event_subscription)
 #define VIDIOC_UNSUBSCRIBE_EVENT _IOW('V', 91, struct v4l2_event_subscription)
 #define VIDIOC_STREAMON _IOW('V', 18, int)
 #define VIDIOC_DQBUF _IOWR('V', 17, struct v4l2_buffer)

 #define V4L2_PIX_FMT_YUYV v4l2_fourcc('Y', 'U', 'Y', 'V')/* 16  YUV 4:2:2     */
 #define V4L2_PIX_FMT_MJPEG v4l2_fourcc('M', 'J', 'P', 'G')/* Motion-JPEG   */
 #define V4L2_PIX_FMT_JPEG v4l2_fourcc('J', 'P', 'E', 'G')/* JFIF JPEG     */
 #define V4L2_PIX_FMT_DV v4l2_fourcc('d', 'v', 's', 'd')/* 1394          */
 #define V4L2_PIX_FMT_MPEG v4l2_fourcc('M', 'P', 'E', 'G')/* MPEG-1/2/4 Multiplexed */
 #define V4L2_PIX_FMT_H264 v4l2_fourcc('H', '2', '6', '4')/* H264 with start codes */
 #define V4L2_PIX_FMT_H264_NO_SC v4l2_fourcc('A', 'V', 'C', '1')/* H264 without start codes */
 #define V4L2_PIX_FMT_H264_MVC v4l2_fourcc('M', '2', '6', '4')/* H264 MVC */
 #define V4L2_PIX_FMT_H263 v4l2_fourcc('H', '2', '6', '3')/* H263          */
 #define V4L2_PIX_FMT_MPEG1 v4l2_fourcc('M', 'P', 'G', '1')/* MPEG-1 ES     */
 #define V4L2_PIX_FMT_MPEG2 v4l2_fourcc('M', 'P', 'G', '2')/* MPEG-2 ES     */
 #define V4L2_PIX_FMT_MPEG4 v4l2_fourcc('M', 'P', 'G', '4')/* MPEG-4 ES     */
 #define V4L2_PIX_FMT_XVID v4l2_fourcc('X', 'V', 'I', 'D')/* Xvid           */
 #define V4L2_PIX_FMT_VC1_ANNEX_G v4l2_fourcc('V', 'C', '1', 'G')/* SMPTE 421M Annex G compliant stream */
 #define V4L2_PIX_FMT_VC1_ANNEX_L v4l2_fourcc('V', 'C', '1', 'L')/* SMPTE 421M Annex L compliant stream */
 #define V4L2_PIX_FMT_VP8 v4l2_fourcc('V', 'P', '8', '0')/* VP8 */

 #define V4L2_EVENT_PRIVATE_START 0x08000000
 #define UVC_EVENT_FIRST (V4L2_EVENT_PRIVATE_START + 0)
 #define UVC_EVENT_CONNECT (V4L2_EVENT_PRIVATE_START + 0)
 #define UVC_EVENT_DISCONNECT (V4L2_EVENT_PRIVATE_START + 1)
 #define UVC_EVENT_STREAMON (V4L2_EVENT_PRIVATE_START + 2)
 #define UVC_EVENT_STREAMOFF (V4L2_EVENT_PRIVATE_START + 3)
 #define UVC_EVENT_SETUP (V4L2_EVENT_PRIVATE_START + 4)
 #define UVC_EVENT_DATA (V4L2_EVENT_PRIVATE_START + 5)
 #define UVC_EVENT_LAST (V4L2_EVENT_PRIVATE_START + 5)

/*struct uvc_streaming_control {
    __u16 bmHint;
    __u8  bFormatIndex;
    __u8  bFrameIndex;
    __u32 dwFrameInterval;
    __u16 wKeyFrameRate;
    __u16 wPFrameRate;
    __u16 wCompQuality;
    __u16 wCompWindowSize;
    __u16 wDelay;
    __u32 dwMaxVideoFrameSize;
    __u32 dwMaxPayloadTransferSize;
    __u32 dwClockFrequency;
    __u8  bmFramingInfo;
    __u8  bPreferedVersion;
    __u8  bMinVersion;
    __u8  bMaxVersion;
} __attribute__((__packed__));
 */
struct v4l2_event_subscription
{
    __u32 type;
    __u32 id;
    __u32 flags;
    __u32 reserved[5];
};

/* Payload for V4L2_EVENT_VSYNC */
struct v4l2_event_vsync
{
    /* Can be V4L2_FIELD_ANY, _NONE, _TOP or _BOTTOM */
    __u8 field;
} __attribute__ ((packed));

/* Payload for V4L2_EVENT_CTRL */
 #define V4L2_EVENT_CTRL_CH_VALUE (1 << 0)
 #define V4L2_EVENT_CTRL_CH_FLAGS (1 << 1)
 #define V4L2_EVENT_CTRL_CH_RANGE (1 << 2)

struct v4l2_event_ctrl
{
    __u32 changes;
    __u32 type;
    union
    {
        __s32 value;
        __s64 value64;
    };
    __u32 flags;
    __s32 minimum;
    __s32 maximum;
    __s32 step;
    __s32 default_value;
};

struct v4l2_event_frame_sync
{
    __u32 frame_sequence;
};

struct v4l2_event
{
    __u32 type;
    union
    {
        struct v4l2_event_vsync      vsync;
        struct v4l2_event_ctrl       ctrl;
        struct v4l2_event_frame_sync frame_sync;
        __u8                         data[64];
    }               u;
    __u32           pending;
    __u32           sequence;
    struct timespec timestamp;
    __u32           id;
    __u32           reserved[8];
};
enum v4l2_buf_type
{
    V4L2_BUF_TYPE_VIDEO_CAPTURE = 1,
    V4L2_BUF_TYPE_VIDEO_OUTPUT  = 2,
    V4L2_BUF_TYPE_VIDEO_OVERLAY = 3,
    V4L2_BUF_TYPE_VBI_CAPTURE = 4,
    V4L2_BUF_TYPE_VBI_OUTPUT = 5,
    V4L2_BUF_TYPE_SLICED_VBI_CAPTURE = 6,
    V4L2_BUF_TYPE_SLICED_VBI_OUTPUT = 7,
 #if 1
    /* Experimental */
    V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
 #endif
    V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE = 10,

    /* Deprecated, do not use */
    V4L2_BUF_TYPE_PRIVATE = 0x80,
};

enum v4l2_memory
{
    V4L2_MEMORY_MMAP = 1,
    V4L2_MEMORY_USERPTR = 2,
    V4L2_MEMORY_OVERLAY = 3,
    V4L2_MEMORY_DMABUF = 4,
};

enum v4l2_field
{
    V4L2_FIELD_ANY  = 0,          /* driver can choose from none,
                                  top, bottom, interlaced
                                  depending on whatever it thinks
                                  is approximate ... */
    V4L2_FIELD_NONE = 1, /* this device has no fields ... */
    V4L2_FIELD_TOP = 2, /* top field only */
    V4L2_FIELD_BOTTOM = 3, /* bottom field only */
    V4L2_FIELD_INTERLACED = 4, /* both fields interlaced */
    V4L2_FIELD_SEQ_TB = 5,        /* both fields sequential into one
                                  buffer, top-bottom order */
    V4L2_FIELD_SEQ_BT = 6, /* same as above + bottom-top order */
    V4L2_FIELD_ALTERNATE = 7,     /* both fields alternating into
                                  separate buffers */
    V4L2_FIELD_INTERLACED_TB = 8, /* both fields interlaced, top field
                                  first and the top field is
                                  transmitted first */
    V4L2_FIELD_INTERLACED_BT = 9, /* both fields interlaced, top field
                                  first and the bottom field is
                                  transmitted first */
};
struct v4l2_capability
{
    __u8  driver[16];
    __u8  card[32];
    __u8  bus_info[32];
    __u32 version;
    __u32 capabilities;
    __u32 device_caps;
    __u32 reserved[3];
};
struct v4l2_timecode
{
    __u32 type;
    __u32 flags;
    __u8  frames;
    __u8  seconds;
    __u8  minutes;
    __u8  hours;
    __u8  userbits[4];
};
struct v4l2_buffer
{
    __u32                index;
    __u32                type;
    __u32                bytesused;
    __u32                flags;
    __u32                field;
    struct timeval       timestamp;
    struct v4l2_timecode timecode;
    __u32                sequence;

    /* memory location */
    __u32 memory;
    union
    {
        __u32              offset;
        unsigned long      userptr;
        struct v4l2_plane* planes;
        __s32              fd;
    }     m;
    __u32 length;
    __u32 reserved2;
    __u32 reserved;
};
struct v4l2_requestbuffers
{
    __u32 count;
    __u32 type;           /* enum v4l2_buf_type */
    __u32 memory;         /* enum v4l2_memory */
    __u32 reserved[2];
};
struct v4l2_pix_format
{
    __u32 width;
    __u32 height;
    __u32 pixelformat;
    __u32 field;          /* enum v4l2_field */
    __u32 bytesperline;   /* for padding, zero if unused */
    __u32 sizeimage;
    __u32 colorspace;     /* enum v4l2_colorspace */
    __u32 priv;           /* private data, depends on pixelformat */
};
 #define VIDEO_MAX_PLANES 8
struct v4l2_plane_pix_format
{
    __u32 sizeimage;
    __u16 bytesperline;
    __u16 reserved[7];
} __attribute__ ((packed));
struct v4l2_pix_format_mplane
{
    __u32 width;
    __u32 height;
    __u32 pixelformat;
    __u32 field;
    __u32 colorspace;

    struct v4l2_plane_pix_format plane_fmt[VIDEO_MAX_PLANES];
    __u8                         num_planes;
    __u8                         reserved[11];
} __attribute__ ((packed));
struct v4l2_rect
{
    __s32 left;
    __s32 top;
    __s32 width;
    __s32 height;
};
 #define __user
struct v4l2_clip
{
    struct v4l2_rect c;
    struct v4l2_clip __user* next;
};
struct v4l2_window
{
    struct v4l2_rect w;
    __u32            field;          /* enum v4l2_field */
    __u32            chromakey;
    struct v4l2_clip __user* clips;
    __u32            clipcount;
    void __user*     bitmap;
    __u8             global_alpha;
};
struct v4l2_vbi_format
{
    __u32 sampling_rate;          /* in 1 Hz */
    __u32 offset;
    __u32 samples_per_line;
    __u32 sample_format;          /* V4L2_PIX_FMT_* */
    __s32 start[2];
    __u32 count[2];
    __u32 flags;                  /* V4L2_VBI_* */
    __u32 reserved[2];            /* must be zero */
};
struct v4l2_sliced_vbi_format
{
    __u16 service_set;

    /* service_lines[0][...] specifies lines 0-23 (1-23 used) of the first field
       service_lines[1][...] specifies lines 0-23 (1-23 used) of the second field
       (equals frame lines 313-336 for 625 line video
       standards, 263-286 for 525 line standards) */
    __u16 service_lines[2][24];
    __u32 io_size;
    __u32 reserved[2];            /* must be zero */
};
struct v4l2_format
{
    __u32 type;
    union
    {
        struct v4l2_pix_format        pix;       /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
        struct v4l2_pix_format_mplane pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
        struct v4l2_window            win;       /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
        struct v4l2_vbi_format        vbi;       /* V4L2_BUF_TYPE_VBI_CAPTURE */
        struct v4l2_sliced_vbi_format sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
        __u8                          raw_data[200]; /* user-defined */
    } fmt;
};
#endif


#define clamp(val, min, max) ({                 \
                                  typeof(val)__val = (val);              \
                                  typeof(min)__min = (min);              \
                                  typeof(max)__max = (max);              \
                                  (void) (&__val == &__min);              \
                                  (void) (&__val == &__max);              \
                                  __val = __val < __min ? __min : __val;   \
                                  __val > __max ? __max : __val; })

#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum
{
    IO_METHOD_MMAP = 0x1,
    IO_METHOD_USERPTR = 0x2,
} io_method;

// H264 extension unit id, must same as to driver
#define UNIT_XU_H264 (10)

//USB_Video_Payload_H_264_1.0 3.3
#define UVCX_PICTURE_TYPE_CONTROL 0x09

// HICAMERA
#define UNIT_XU_HICAMERA (0x11)


typedef struct uvc_probe_t
{
    unsigned char set;
    unsigned char get;
    unsigned char max;
    unsigned char min;
} uvc_probe_t;

struct uvc_device
{
    int fd;
    //struct uvc_streaming_control probe;
    //struct uvc_streaming_control commit;
    int control;
    int unit_id;
    int interface_id;
    unsigned int fcc;
    unsigned int width;
    unsigned int height;
    unsigned int    nbufs;
    unsigned int bulk;
    uint8_t      color;
    unsigned int imgsize;
    unsigned int bulk_size;
    uvc_probe_t  probe_status;
    int streaming;

	void **mem;
	unsigned int bufsize;
};
enum usb_connect_state
{
	USB_STATE_DISCONNECT,
	USB_STATE_CONNECT,
	USB_STATE_CONNECT_RESET,
};
#define UVCIOC_USB_CONNECT_STATE                _IOW('U', 2, int)
int fd=-1;
int reset_usb()
{
	int ret=0,type=0;
    if (fd<0)
    {
        return -1;
    }
    	//重新连接USB
	usleep(10 * 1000);
	type = USB_STATE_CONNECT_RESET;
	
	if ((ret = ioctl(fd, UVCIOC_USB_CONNECT_STATE, &type)) < 0){
		printf("Unable to set USB_STATE_CONNECT_RESET");	
		return -1;
	}
}

int main(int argc, char* argv[])
{
    fd = open("/dev/video0", O_RDWR | O_NONBLOCK);

    if (fd == -1)
    {
        printf("v4l2 open failed");
        return -1;
    }
	reset_usb();
}