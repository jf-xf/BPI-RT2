#ifndef __RTW_ECOS_PROC_H__
#define __RTW_ECOS_PROC_H__

enum proc_ops {
	READ=1,
	WRITE,
	READ_WRITE
};

#define RTW_PROC_HDL_SSEQ(_name, _read, _write) \
	{ .name = _name, .read = _read, .write = _write}

#define RTW_PROC_HDL_SEQ(_name, _read, _write) \
	{ .name = _name, .read = _read, .write = _write}

#define RTW_PROC_HDL_SZSEQ(_name, _read, _write, _size) \
	{ .name = _name, .read = _read, .write = _write}

struct rtw_proc_hdl {
	char *name;
	int (*read)(struct seq_file *, void *);
	ssize_t (*write)(struct file *, const char *, size_t , loff_t *, void *);
	u8 type;
};

#endif
