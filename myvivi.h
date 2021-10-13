
typedef int (*fmtfunc)(struct file *file, void *priv, struct v4l2_format *f);

void fmt_sp2mp(const struct v4l2_format *sp_fmt, struct v4l2_format *mp_fmt);
int fmt_sp2mp_func(struct file *file, void *priv,
		   struct v4l2_format *f, fmtfunc func);
