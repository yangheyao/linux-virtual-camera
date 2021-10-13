#include <linux/init.h>
#include <linux/module.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <linux/videodev2.h>
#include <media/videobuf2-vmalloc.h>
#include <media/videobuf2-v4l2.h>
#include <linux/platform_device.h>
#include <linux/timer.h>

#include "image.h"

#include "myvivi.h"
/*
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
	#define VFL_TYPE_VIDEO VFL_TYPE_GRABBER
#endif
*/

/* The minimum image width/height */
//#define MIN_WIDTH  48
//#define MIN_HEIGHT 32
#define MIN_WIDTH  1920
#define MIN_HEIGHT 1080

#define MAX_WIDTH 1920
//#define MAX_HEIGHT 1200
#define MAX_HEIGHT 1080

struct myvivi_fmt {
	u32	fourcc;          /* v4l2 format id */
	u8	buffers;
	u32	bit_depth;
	
	u32	colorspace;

};

struct myvivi_buffer {
	struct vb2_v4l2_buffer vb;
	struct list_head	list;
};

struct vivi {
	struct v4l2_device 		v4l2_dev;
	struct video_device 	vid_cap_dev;
	struct vb2_queue 		vb_vid_cap_q;
	const struct myvivi_fmt	*fmt_cap;
	
	u32 					vid_cap_caps;
	spinlock_t				slock;
	struct mutex 			mutex;
	
	struct v4l2_rect		fmt_cap_rect;
	enum v4l2_field			field_cap;
	unsigned				bytesperline;
	
	struct list_head		vid_cap_active;
	
	struct timer_list 		timer;
};

static struct vivi *myvivi;



struct myvivi_fmt myvivi_formats = {
	//.fourcc   = V4L2_PIX_FMT_YUYV,
	//.bit_depth = 16,
	//.buffers = 1,
	//.colorspace = V4L2_COLORSPACE_SRGB,
	
	//.fourcc   = V4L2_PIX_FMT_RGB565, // gggbbbbb rrrrrggg 
	//.bit_depth = 16,
	//.buffers = 1,
	//.colorspace = V4L2_COLORSPACE_SRGB,

	.fourcc   = V4L2_PIX_FMT_HSV32, /* HSV 32bits */
	//.color_enc = TGP_COLOR_ENC_HSV,
	//.vdownsampling = { 1 },
	.colorspace = V4L2_COLORSPACE_SRGB,
	.bit_depth = 32,
	//.planes   = 1,
	.buffers = 1,	
};

#if 0
struct vivid_fmt vivid_format[] = {
//struct myvivi_fmt myvivi_formats[] = {
	{
		.fourcc   = V4L2_PIX_FMT_YUYV,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 1,
		.buffers = 1,
		.data_offset = { PLANE0_DATA_OFFSET },
	},
	{
		.fourcc   = V4L2_PIX_FMT_UYVY,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YVYU,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_VYUY,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV422P,
		.vdownsampling = { 1, 1, 1 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV420,
		.vdownsampling = { 1, 2, 2 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YVU420,
		.vdownsampling = { 1, 2, 2 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV12,
		.vdownsampling = { 1, 2 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV21,
		.vdownsampling = { 1, 2 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV16,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV61,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV24,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV42,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 16 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV555, /* uuuvvvvv ayyyyyuu */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0x8000,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV565, /* uuuvvvvv yyyyyuuu */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV444, /* uuuuvvvv aaaayyyy */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0xf000,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV32, /* ayuv */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0x000000ff,
	},
	{
		.fourcc   = V4L2_PIX_FMT_GREY,
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.color_enc = TGP_COLOR_ENC_LUMA,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_Y10,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_LUMA,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_Y12,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_LUMA,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_Y16,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_LUMA,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_Y16_BE,
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.color_enc = TGP_COLOR_ENC_LUMA,
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB332, /* rrrgggbb */
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB565, /* gggbbbbb rrrrrggg */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.can_do_overlay = true,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB565X, /* rrrrrggg gggbbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.can_do_overlay = true,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB444, /* xxxxrrrr ggggbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_XRGB444, /* xxxxrrrr ggggbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_ARGB444, /* aaaarrrr ggggbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0x00f0,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB555, /* gggbbbbb xrrrrrgg */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.can_do_overlay = true,
	},
	{
		.fourcc   = V4L2_PIX_FMT_XRGB555, /* gggbbbbb xrrrrrgg */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.can_do_overlay = true,
	},
	{
		.fourcc   = V4L2_PIX_FMT_ARGB555, /* gggbbbbb arrrrrgg */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.can_do_overlay = true,
		.alpha_mask = 0x8000,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB555X, /* xrrrrrgg gggbbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_XRGB555X, /* xrrrrrgg gggbbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_ARGB555X, /* arrrrrgg gggbbbbb */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0x0080,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB24, /* rgb */
		.vdownsampling = { 1 },
		.bit_depth = { 24 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_BGR24, /* bgr */
		.vdownsampling = { 1 },
		.bit_depth = { 24 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_BGR666, /* bbbbbbgg ggggrrrr rrxxxxxx */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_RGB32, /* xrgb */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_BGR32, /* bgrx */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_XRGB32, /* xrgb */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_XBGR32, /* bgrx */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_ARGB32, /* argb */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0x000000ff,
	},
	{
		.fourcc   = V4L2_PIX_FMT_ABGR32, /* bgra */
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
		.alpha_mask = 0xff000000,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SBGGR8, /* Bayer BG/GR */
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGBRG8, /* Bayer GB/RG */
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGRBG8, /* Bayer GR/BG */
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SRGGB8, /* Bayer RG/GB */
		.vdownsampling = { 1 },
		.bit_depth = { 8 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SBGGR10, /* Bayer BG/GR */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGBRG10, /* Bayer GB/RG */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGRBG10, /* Bayer GR/BG */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SRGGB10, /* Bayer RG/GB */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SBGGR12, /* Bayer BG/GR */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGBRG12, /* Bayer GB/RG */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SGRBG12, /* Bayer GR/BG */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_SRGGB12, /* Bayer RG/GB */
		.vdownsampling = { 1 },
		.bit_depth = { 16 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_HSV24, /* HSV 24bits */
		.color_enc = TGP_COLOR_ENC_HSV,
		.vdownsampling = { 1 },
		.bit_depth = { 24 },
		.planes   = 1,
		.buffers = 1,
	},
	{
		.fourcc   = V4L2_PIX_FMT_HSV32, /* HSV 32bits */
		.color_enc = TGP_COLOR_ENC_HSV,
		.vdownsampling = { 1 },
		.bit_depth = { 32 },
		.planes   = 1,
		.buffers = 1,
	},

	/* Multiplanar formats */

	{
		.fourcc   = V4L2_PIX_FMT_NV16M,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 2,
		.data_offset = { PLANE0_DATA_OFFSET, 0 },
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV61M,
		.vdownsampling = { 1, 1 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 2,
		.data_offset = { 0, PLANE0_DATA_OFFSET },
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV420M,
		.vdownsampling = { 1, 2, 2 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YVU420M,
		.vdownsampling = { 1, 2, 2 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV12M,
		.vdownsampling = { 1, 2 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 2,
	},
	{
		.fourcc   = V4L2_PIX_FMT_NV21M,
		.vdownsampling = { 1, 2 },
		.bit_depth = { 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 2,
		.buffers = 2,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV422M,
		.vdownsampling = { 1, 1, 1 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YVU422M,
		.vdownsampling = { 1, 1, 1 },
		.bit_depth = { 8, 4, 4 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YUV444M,
		.vdownsampling = { 1, 1, 1 },
		.bit_depth = { 8, 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
	{
		.fourcc   = V4L2_PIX_FMT_YVU444M,
		.vdownsampling = { 1, 1, 1 },
		.bit_depth = { 8, 8, 8 },
		.color_enc = TGP_COLOR_ENC_YCBCR,
		.planes   = 3,
		.buffers = 3,
	},
};
#endif


static void fillbuff(char *vbuf, int bw, int bh){
	static u8 count = 0;
	static u8 c = 0;
	int w,h;
	int w_ofset = 0,h_ofset = 0;
	int i,j;
	
	if(bw > (IMAGE_WIDTH*2)) {
		w = IMAGE_WIDTH * 2;
		w_ofset = (bw - w) / 2;
	} else {
		w = bw;
	}
	
	if(bh > IMAGE_HEIGHT) {
		h = IMAGE_HEIGHT;
		h_ofset = (bh - h) / 2;
	} else {
		h = bh;
	}
	
	for(i=0;i<h;i++){
		for(j=0;j<w;j++){
			vbuf[bw*(i+h_ofset) + j+w_ofset] = image[count][w*i + j];
		}
	}
	
	
	//太快了, 慢点
	c++;
	c = c % 10;
	if(c == 0) {
		count++;
		count = count % IMAGE_NUM;
	}
}

static void myvivi_timer_function(struct timer_list *t){
	struct vivi *vind = container_of(t, struct vivi, timer);
    struct myvivi_buffer *vid_cap_buf = NULL;
	char *vbuf;
	
	printk("------%s----\n",__func__);
	printk("width = %d,height = %d\n",vind->fmt_cap_rect.width,vind->fmt_cap_rect.height);
	
	if (!list_empty(&vind->vid_cap_active)) {
		vid_cap_buf = list_entry(vind->vid_cap_active.next, struct myvivi_buffer, list);
		if(vid_cap_buf->vb.vb2_buf.state != VB2_BUF_STATE_ACTIVE) {
			printk(KERN_ERR"buffer no active,error!!!\n");
			return;
		}
		list_del(&vid_cap_buf->list);
	}else {
		printk("No active queue to serve\n");
        goto out;
	}
    
	//取buf
	vbuf = vb2_plane_vaddr(&vid_cap_buf->vb.vb2_buf, 0);
	printk("bytesperline=%d\n",vind->bytesperline);
	
	//填充数据
	memset(vbuf,0xff,vind->bytesperline * vind->fmt_cap_rect.height);
	fillbuff(vbuf,vind->bytesperline,vind->fmt_cap_rect.height);
	
    // 它干两个工作，把buffer 挂入done_list 另一个唤醒应用层序，让它dqbuf
    vb2_buffer_done(&vid_cap_buf->vb.vb2_buf, VB2_BUF_STATE_DONE);
    
out:
    //修改timer的超时时间 : 30fps, 1秒里有30帧数据,每1/30 秒产生一帧数据
    mod_timer(&vind->timer, jiffies + HZ/30);
}



//表示它是一个摄像头设备
static int myvivi_vidoc_querycap(struct file *file,void *priv,
					struct v4l2_capability *cap) {
	struct vivi *vind = video_drvdata(file);

	strcpy(cap->driver, "myvivi");
	strcpy(cap->card, "myvivi");
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:%s", vind->v4l2_dev.name);

	cap->capabilities = vind->vid_cap_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

//列举支持哪些格式
static int myvivi_vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_fmtdesc *f){
	//struct vivi *vind = video_drvdata(file);
	const struct myvivi_fmt *fmt;

	printk("-----%s:line=%d, f->index=%d\n",__func__,__LINE__,f->index);

	if (f->index >= 1){
	  printk("jonah %s return -EINVAL\n",__func__);
		return -EINVAL;
	}
	//if ( f->index >= ARRAY_SIZE(vivid_formats) - (dev->multiplanar ? 0 : VIVID_MPLANAR_FORMATS)){
	/*
	if( f->index >= ARRAY_SIZE(myvivi_formats)){
	  return -EINVAL;
	}
	*/

	fmt = &myvivi_formats;
	//fmt = &myvivi_formats[f->index];

	f->pixelformat = fmt->fourcc;
	return 0;
}

/* 返回当前所使用的格式 */
static int myvivi_vidioc_g_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct vivi *vind = video_drvdata(file);

	printk("-----%s:line=%d\n",__func__,__LINE__);
	
	pix->width = vind->fmt_cap_rect.width;
	pix->height = vind->fmt_cap_rect.height;
	pix->pixelformat = vind->fmt_cap->fourcc;
	pix->field = V4L2_FIELD_NONE;
	pix->colorspace = vind->fmt_cap->colorspace;
	pix->bytesperline = vind->bytesperline;
	pix->sizeimage = pix->bytesperline * pix->width;
	
	
	return 0;
}

//返回当前所使用的格式
static int myvivi_vidioc_try_fmt_vid_cap(struct file *file,void *priv, 
			struct v4l2_format *f) {
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct vivi *vind = video_drvdata(file);
	u32 w;
	u32 bit_depth;
	
	printk("-----%s:line=%d\n",__func__,__LINE__);

	v4l_bound_align_image(&pix->width, MIN_WIDTH, MAX_WIDTH, 2,
                  &pix->height, MIN_HEIGHT, MAX_HEIGHT, 0, 0);
	
	pix->pixelformat = vind->fmt_cap->fourcc;
	pix->field = V4L2_FIELD_NONE;
	pix->colorspace = vind->fmt_cap->colorspace;
	w = pix->width;
	bit_depth = vind->fmt_cap->bit_depth;
	pix->bytesperline = (w * bit_depth) >> 3;   
	pix->sizeimage = pix->bytesperline * pix->width;
	
	
	
	return 0;
}

/*jonah add for fmt_sp2mp func*/
void fmt_sp2mp(const struct v4l2_format *sp_fmt, struct v4l2_format *mp_fmt)
{
  struct v4l2_pix_format_mplane *mp = &mp_fmt->fmt.pix_mp;
  struct v4l2_plane_pix_format  *ppix = &mp->plane_fmt[0];
  const struct v4l2_pix_format *pix = &sp_fmt->fmt.pix;
  bool is_out = sp_fmt->type == V4L2_BUF_TYPE_VIDEO_OUTPUT;

  memset(mp->reserved, 0, sizeof(mp->reserved));
  mp_fmt->type = is_out ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
    V4L2_CAP_VIDEO_CAPTURE_MPLANE;

  mp->width = pix->width;
  mp->height = pix->height;
  mp->pixelformat = pix->pixelformat;
  mp->field = pix->field;
  mp->colorspace = pix->colorspace;
  mp->xfer_func = pix->xfer_func;

  /*Also copies hsv_enc*/
  mp->ycbcr_enc = pix->ycbcr_enc;
  mp->quantization = pix->quantization;
  mp->num_planes = 1;
  mp->flags = pix->flags;
  ppix->sizeimage = pix->sizeimage;
  ppix->bytesperline = pix->bytesperline;

  memset(ppix->reserved, 0, sizeof(ppix->reserved));
}

int fmt_sp2mp_func(struct file *file, void *priv,
		   struct v4l2_format *f, fmtfunc func)
{
  struct v4l2_format fmt;
  struct v4l2_pix_format_mplane *mp = &fmt.fmt.pix_mp;
  struct v4l2_plane_pix_format *ppix = &mp->plane_fmt[0];
  struct v4l2_pix_format *pix = &f->fmt.pix;
  int ret;

  /* Converts to a mplane format */
  fmt_sp2mp(f, &fmt);
  /* Passes it to the generic mplane format function */
  ret = func(file, priv, &fmt);
  /* Copies back the mplane data to the single plane format */
  pix->width = mp->width;
  pix->height = mp->height;
  pix->pixelformat = mp->pixelformat;
  pix->field = mp->field;
  pix->colorspace = mp->colorspace;
  pix->xfer_func = mp->xfer_func;
  /* Also copies hsv_enc */
  pix->ycbcr_enc = mp->ycbcr_enc;
  pix->quantization = mp->quantization;
  pix->sizeimage = ppix->sizeimage;
  pix->bytesperline = ppix->bytesperline;
  pix->flags = mp->flags;


  return ret;
}

//设置数据格式
static int myvivi_vidioc_s_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f) {
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct vivi *vind = video_drvdata(file);

	int ret = myvivi_vidioc_try_fmt_vid_cap(file, priv, f);
	printk("jonah %s-%d\n",__func__,__LINE__);
    if (ret < 0) {
		printk(KERN_ERR"try format error!!!\n");
        return ret;
	}
	
	vind->fmt_cap_rect.width = pix->width;
	vind->fmt_cap_rect.height = pix->height;
	vind->bytesperline = pix->bytesperline;
	
	return 0;
}

static int myvivi_vidioc_g_fbuf(struct file *file, void *fh, struct v4l2_framebuffer *a)
{
	return 0;
}

static int myvivi_vidioc_s_fbuf(struct file *file, void *fh, const struct v4l2_framebuffer *a)
{
	return 0;
}


static const struct v4l2_ioctl_ops myvivi_ioctl_ops = {
	//表示它是一个摄像头设备
	.vidioc_querycap = myvivi_vidoc_querycap,

	//用于列举、获取、测试、设置摄像头的数据格式
	.vidioc_enum_fmt_vid_cap 	= myvivi_vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap 		= myvivi_vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap 	= myvivi_vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap 		= myvivi_vidioc_s_fmt_vid_cap,

	
	//.vidioc_enum_framesizes		= vidioc_enum_framesizes,
	.vidioc_g_fbuf			= myvivi_vidioc_g_fbuf,
	.vidioc_s_fbuf			= myvivi_vidioc_s_fbuf,

	//缓冲区操作: 申请/查询/放入/取出队列
	.vidioc_reqbufs 			= vb2_ioctl_reqbufs,
	.vidioc_querybuf 			= vb2_ioctl_querybuf,
	.vidioc_qbuf 				= vb2_ioctl_qbuf,
	.vidioc_dqbuf 				= vb2_ioctl_dqbuf,

	//启动/停止
	.vidioc_streamon 			= vb2_ioctl_streamon,
	.vidioc_streamoff 			= vb2_ioctl_streamoff,
};

static int myvivi_fop_release(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	
	if (vdev->queue)
		return vb2_fop_release(file);
	return v4l2_fh_release(file);
}

static const struct v4l2_file_operations myvivi_fops = {
	.owner			= THIS_MODULE,
	.open           = v4l2_fh_open,
	.release        = myvivi_fop_release,
	.poll			= vb2_fop_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap           = vb2_fop_mmap,
};

// vb2 核心层 vb2_reqbufs 中调用它，确定申请缓冲区的大小
static int vid_cap_queue_setup(struct vb2_queue *vq,
		       unsigned *nbuffers, unsigned *nplanes,
		       unsigned sizes[], struct device *alloc_devs[]){
	struct vivi *vind = vb2_get_drv_priv(vq);
	unsigned buffers = vind->fmt_cap->buffers;
	
	printk("width = %d \n",vind->fmt_cap_rect.width);
    printk("height = %d \n",vind->fmt_cap_rect.height);
    printk("pixelsize = %d \n",vind->bytesperline);
	printk("buffers = %d \n",buffers);
	
	sizes[0] = vind->bytesperline * vind->fmt_cap_rect.height;
	
	if (vq->num_buffers + *nbuffers < 2)
		*nbuffers = 2 - vq->num_buffers;
	
	*nplanes = buffers;
	
	printk("%s: count=%d\n", __func__, *nbuffers);
	
	return 0;
}

//APP调用ioctlVIDIOC_QBUF时导致此函数被调用
static int vid_cap_buf_prepare(struct vb2_buffer *vb){
	struct vivi *vind = vb2_get_drv_priv(vb->vb2_queue);
	unsigned long size;

	size = vind->bytesperline * vind->fmt_cap_rect.height;;

	if (vb2_plane_size(vb, 0) < size) {
		printk(KERN_ERR"%s data will not fit into plane (%lu < %lu)\n",
				__func__ ,vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(vb, 0, size);
	
	return 0;
}

static void vid_cap_buf_finish(struct vb2_buffer *vb) {
	
}

//APP调用ioctl VIDIOC_QBUF时
static void vid_cap_buf_queue(struct vb2_buffer *vb) {
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct vivi *vind = vb2_get_drv_priv(vb->vb2_queue);
	struct myvivi_buffer *buf = container_of(vbuf, struct myvivi_buffer, vb);

	printk("%s\n", __func__);

	spin_lock(&vind->slock);
	//把buf放入本地一个队列尾部,定时器处理函数就可以从本地队列取出videobuf
	list_add_tail(&buf->list, &vind->vid_cap_active);
	spin_unlock(&vind->slock);
}

static int vid_cap_start_streaming(struct vb2_queue *vq, unsigned count) {
	struct vivi *vind = vb2_get_drv_priv(vq);
	printk("------start timer-----\n");
	timer_setup(&vind->timer, myvivi_timer_function, 0);
	vind->timer.expires = jiffies + HZ/2;
	add_timer(&vind->timer);
	
	return 0;
}

static void vid_cap_stop_streaming(struct vb2_queue *vq) {
	struct vivi *vind = vb2_get_drv_priv(vq);

	printk("%s\n", __func__);
	del_timer(&vind->timer);
	
	/* Release all active buffers */
	while (!list_empty(&vind->vid_cap_active)) {
		struct myvivi_buffer *buf;

		buf = list_entry(vind->vid_cap_active.next,
				 struct myvivi_buffer, list);
		list_del(&buf->list);
		vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_ERROR);
		printk("vid_cap buffer %d stop\n",buf->vb.vb2_buf.index);
	}
	
}

const struct vb2_ops myvivi_vid_cap_qops = {
	.queue_setup		= vid_cap_queue_setup,
	.buf_prepare		= vid_cap_buf_prepare,
	.buf_finish			= vid_cap_buf_finish,
	.buf_queue			= vid_cap_buf_queue,
	.start_streaming	= vid_cap_start_streaming,
	.stop_streaming		= vid_cap_stop_streaming,
};

static void myvivi_dev_release(struct v4l2_device *v4l2_dev)
{
	struct vivi *vind = container_of(v4l2_dev, struct vivi, v4l2_dev);

	v4l2_device_unregister(&vind->v4l2_dev);
	kfree(vind);
}

void video_device_release_empty(struct video_device *vdev) {

}

static int myvivi_probe(struct platform_device *pdev) {
	int ret = -1;
	struct vb2_queue *q;
	struct video_device *vfd;
	
	myvivi = kzalloc(sizeof(*myvivi), GFP_KERNEL);
	if (!myvivi)
		return -ENOMEM;
	
	/* 0.注册v4l2_dev */
	snprintf(myvivi->v4l2_dev.name, sizeof(myvivi->v4l2_dev.name),
			"%s-00", "myvivi");
	ret = v4l2_device_register(&pdev->dev,&myvivi->v4l2_dev);
	if (ret < 0) {
		printk(KERN_ERR"Failed to register v4l2_device: %d\n", ret);
		goto v4l2_dev_err;
	}
	myvivi->v4l2_dev.release = myvivi_dev_release;

	myvivi->vid_cap_caps = 	V4L2_CAP_VIDEO_CAPTURE | \
							V4L2_CAP_VIDEO_OVERLAY | \
							V4L2_CAP_STREAMING;
	
	myvivi->fmt_cap = &myvivi_formats;
	
	/* initialize locks */
	spin_lock_init(&myvivi->slock);
	mutex_init(&myvivi->mutex);

	/* init dma queues */
	INIT_LIST_HEAD(&myvivi->vid_cap_active);
	
	
	/* initialize vid_cap queue */
	q = &myvivi->vb_vid_cap_q;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF | VB2_READ;
	q->drv_priv = myvivi;
	q->buf_struct_size = sizeof(struct myvivi_buffer);
	q->ops = &myvivi_vid_cap_qops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->lock = &myvivi->mutex;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->dev = myvivi->v4l2_dev.dev;
	ret = vb2_queue_init(q);
	if (ret)
		goto unreg_dev;
	
	vfd = &myvivi->vid_cap_dev;
	snprintf(vfd->name, sizeof(vfd->name),
		 "myvivi-00-vid-cap");
	vfd->fops = &myvivi_fops;
	vfd->ioctl_ops = &myvivi_ioctl_ops;
	vfd->device_caps = myvivi->vid_cap_caps;
	vfd->release = video_device_release_empty;
	vfd->v4l2_dev = &myvivi->v4l2_dev;
	vfd->queue = &myvivi->vb_vid_cap_q;
	vfd->lock = &myvivi->mutex;
	video_set_drvdata(vfd, myvivi);


	ret = video_register_device(vfd, VFL_TYPE_GRABBER, -1);
	//ret = video_register_device(vfd, VFL_TYPE_VIDEO, -1);
	if (ret < 0)
		goto unreg_dev;
	
	
	return ret;

unreg_dev:
	v4l2_device_put(&myvivi->v4l2_dev);
v4l2_dev_err:
	kfree(myvivi);

	return -1;
}

static int myvivi_remove(struct platform_device *pdev){
	printk("-----%s----\n",__func__);
	video_unregister_device(&myvivi->vid_cap_dev);
	v4l2_device_put(&myvivi->v4l2_dev);
	//kfree(myvivi);
	return 0;
}

static void myvivi_pdev_release(struct device *dev)
{
}

static struct platform_device myvivi_pdev = {
	.name			= "myvivi",
	.dev.release	= myvivi_pdev_release,
};

static struct platform_driver myvivi_pdrv = {
	.probe		= myvivi_probe,
	.remove		= myvivi_remove,
	.driver		= {
		.name	= "myvivi",
	},
};

static int __init myvivi_init(void)
{
	int ret;

	ret = platform_device_register(&myvivi_pdev);
	if (ret)
		return ret;

	ret = platform_driver_register(&myvivi_pdrv);
	if (ret)
		platform_device_unregister(&myvivi_pdev);

	return ret;
}

static void __exit myvivi_exit(void)
{
	platform_driver_unregister(&myvivi_pdrv);
	platform_device_unregister(&myvivi_pdev);
}

module_init(myvivi_init);
module_exit(myvivi_exit);
MODULE_LICENSE("GPL");


