#ifndef _SBUNIX_VFS_VFS_H
#define _SBUNIX_VFS_VFS_H

struct super_ops {
    struct vnode *(*alloc_vnode) (struct super_block *sb);
    void (*destroy_vnode) (struct vnode *);
    void (*read_vnode) (struct vnode *);
    void (*dirty_vnode) (struct vnode *);
    void (*write_vnode) (struct vnode *, int);
    void (*put_vnode) (struct vnode *);
    void (*drop_vnode) (struct vnode *);
    void (*delete_vnode) (struct vnode *);
    void (*put_super) (struct super_block *);
    void (*write_super) (struct super_block *);
    int (*sync_fs) (struct super_block *, int);
    void (*write_super_lockfs) (struct super_block *);
    void (*unlockfs) (struct super_block *);
//    int (*statfs) (struct super_block *, struct statfs *);
    int (*remount_fs) (struct super_block *, int *, char *);
    void (*clear_vnode) (struct vnode *);
    void (*umount_begin) (struct super_block *);
//    int (*show_options) (struct seq_file *, struct vfsmount *);
};

struct vnode_ops {
    int (*create) (struct vnode *, struct dentry *,int);
    struct dentry * (*lookup) (struct vnode *, struct dentry *);
    int (*link) (struct dentry *, struct vnode *, struct dentry *);
    int (*unlink) (struct vnode *, struct dentry *);
//    int (*symlink) (struct vnode *, struct dentry *, const char *);
    int (*mkdir) (struct vnode *, struct dentry *, int);
    int (*rmdir) (struct vnode *, struct dentry *);
    int (*mknod) (struct vnode *, struct dentry *, int, dev_t);
    int (*rename) (struct vnode *, struct dentry *,
                   struct vnode *, struct dentry *);
//    int (*readlink) (struct dentry *, char *, int);
//    int (*follow_link) (struct dentry *, struct nameidata *);
//    int (*put_link) (struct dentry *, struct nameidata *);
    void (*truncate) (struct vnode *);
    int (*permission) (struct vnode *, int);
//    int (*setattr) (struct dentry *, struct iattr *);
//    int (*getattr) (struct vfsmount *, struct dentry *, struct kstat *);
//    int (*setxattr) (struct dentry *, const char *,
//                     const void *, size_t, int);
//    ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
//    ssize_t (*listxattr) (struct dentry *, char *, size_t);
//    int (*removexattr) (struct dentry *, const char *);
};

struct dentry_ops {
    int (*d_revalidate) (struct dentry *, int);
//    int (*d_hash) (struct dentry *, struct qstr *);
//    int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
    int (*d_delete) (struct dentry *);
    void (*d_release) (struct dentry *);
    void (*d_vput) (struct dentry *, struct vnode *);
};

struct file_ops {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char *, size_t, loff_t *);
    ssize_t (*aio_read) (struct kiocb *, char *, size_t, loff_t);
    ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
    ssize_t (*aio_write) (struct kiocb *, const char *, size_t, loff_t);
    int (*readdir) (struct file *, void *, filldir_t);
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    int (*ioctl) (struct vnode *, struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    int (*open) (struct vnode *, struct file *);
    int (*flush) (struct file *);
    int (*release) (struct vnode *, struct file *);
    int (*fsync) (struct file *, struct dentry *, int);
    int (*aio_fsync) (struct kiocb *, int);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
    ssize_t (*readv) (struct file *, const struct iovec *,
                      unsigned long, loff_t *);
    ssize_t (*writev) (struct file *, const struct iovec *,
                       unsigned long, loff_t *);
    ssize_t (*sendfile) (struct file *, loff_t *, size_t,
                         read_actor_t, void *);
    ssize_t (*sendpage) (struct file *, struct page *, int,
                         size_t, loff_t *, int);
    unsigned long (*get_unmapped_area) (struct file *, unsigned long,
                                        unsigned long, unsigned long,
                                        unsigned long);
    int (*check_flags) (int flags);
    int (*dir_notify) (struct file *filp, unsigned long arg);
    int (*flock) (struct file *filp, int cmd, struct file_lock *fl);
};


struct super_block {
    struct list_head         s_list;            /* list of all superblocks */
    dev_t                    s_dev;             /* identifier */
    unsigned long            s_blocksize;       /* block size in bytes */
    unsigned char            s_dirty;           /* dirty flag */
    unsigned long            s_maxbytes;        /* max file size */
//    unsigned long           s_type;            /* filesystem type */
    struct super_ops         s_op;              /* superblock methods */
    unsigned long            s_flags;          /* mount flags */
    unsigned long            s_magic;          /* filesystem's magic number */
    struct dentry            *s_root;          /* directory mount point */
//    struct rw_semaphore      s_umount;         /* unmount semaphore */
//    struct semaphore         s_lock;           /* superblock semaphore */
    int                      s_count;          /* superblock ref count */
    int                      s_syncing;        /* filesystem syncing flag */
    int                      s_need_sync_fs;   /* not-yet-synced flag */
//    atomic_t                 s_active;         /* active reference count */
//    void                     *s_security;      /* security module */
//    struct list_head         s_dirty;          /* list of dirty vnodes */
//    struct list_head         s_io;             /* list of writebacks */
//    struct hlist_head        s_anon;           /* anonymous dentries */
//    struct list_head         s_files;          /* list of assigned files */
//    struct block_device      *s_bdev;          /* associated block device */
//    struct list_head         s_instances;      /* instances of this fs */
//    struct quota_info        s_dquot;          /* quota-specific options */
    char                     s_id[32];         /* text name */
//    void                     *s_fs_info;       /* filesystem-specific info */
};

struct vnode {
    struct hlist_node       i_hash;              /* hash list */
    struct list_head        i_list;              /* list of vnodes */
    struct list_head        i_dentry;            /* list of dentries */
    unsigned long           i_ino;               /* vnode number */
    atomic_t                i_count;             /* reference counter */
    umode_t                 i_mode;              /* access permissions */
    unsigned int            i_nlink;             /* number of hard links */
    uid_t                   i_uid;               /* user id of owner */
    gid_t                   i_gid;               /* group id of owner */
    kdev_t                  i_rdev;              /* real device node */
    loff_t                  i_size;              /* file size in bytes */
    struct timespec         i_atime;             /* last access time */
    struct timespec         i_mtime;             /* last modify time */
    struct timespec         i_ctime;             /* last change time */
    unsigned int            i_blkbits;           /* block size in bits */
    unsigned long           i_blksize;           /* block size in bytes */
    unsigned long           i_version;           /* version number */
    unsigned long           i_blocks;            /* file size in blocks */
    unsigned short          i_bytes;             /* bytes consumed */
    spinlock_t              i_lock;              /* spinlock */
    struct rw_semaphore     i_alloc_sem;         /* nests inside of i_sem */
    struct semaphore        i_sem;               /* vnode semaphore */
    struct vnode_ops        *i_op;               /* vnode ops table */
    struct file_ops         *i_fop;              /* default vnode ops */
    struct super_block      *i_sb;               /* associated superblock */
    struct file_lock        *i_flock;            /* file lock list */
    struct address_space    *i_mapping;          /* associated mapping */
    struct address_space    i_data;              /* mapping for device */
    struct dquot            *i_dquot[MAXQUOTAS]; /* disk quotas for vnode */
    struct list_head        i_devices;           /* list of block devices */
    struct pipe_vnode_info  *i_pipe;             /* pipe information */
    struct block_device     *i_bdev;             /* block device driver */
    unsigned long           i_dnotify_mask;      /* directory notify mask */
    struct dnotify_struct   *i_dnotify;          /* dnotify */
    unsigned long           i_state;             /* state flags */
    unsigned long           dirtied_when;        /* first dirtying time */
    unsigned int            i_flags;             /* filesystem flags */
    unsigned char           i_sock;              /* is this a socket? */
    atomic_t                i_writecount;        /* count of writers */
    void                    *i_security;         /* security module */
    __u32                   i_generation;        /* vnode version number */
    union {
        void            *generic_ip;         /* filesystem-specific info */
    } u;
};

struct dentry {
    atomic_t                 d_count;      /* usage count */
    unsigned long            d_vfs_flags;  /* dentry cache flags */
    spinlock_t               d_lock;       /* per-dentry lock */
    struct vnode             *d_vnode;     /* associated vnode */
    struct list_head         d_lru;        /* unused list */
    struct list_head         d_child;      /* list of dentries within */
    struct list_head         d_subdirs;    /* subdirectories */
    struct list_head         d_alias;      /* list of alias vnodes */
    unsigned long            d_time;       /* revalidate time */
    struct dentry_ops        *d_op;        /* dentry ops table */
    struct super_block       *d_sb;        /* superblock of file */
    unsigned int             d_flags;      /* dentry flags */
    int                      d_mounted;    /* is this a mount point? */
    void                     *d_fsdata;    /* filesystem-specific data */
    struct rcu_head          d_rcu;        /* RCU locking */
    struct dcookie_struct    *d_cookie;    /* cookie */
    struct dentry            *d_parent;    /* dentry object of parent */
    struct qstr              d_name;       /* dentry name */
    struct hlist_node        d_hash;       /* list of hash table entries */
    struct hlist_head        *d_bucket;    /* hash bucket */
    unsigned char            d_iname[DNAME_INLINE_LEN_MIN]; /* short name */
};

struct file {
    struct list_head       f_list;        /* list of file objects */
    struct dentry          *f_dentry;     /* associated dentry object */
    struct vfsmount        *f_vfsmnt;     /* associated mounted fs */
    struct file_ops        *f_op;         /* file ops table */
    atomic_t               f_count;       /* file object's usage count */
    unsigned int           f_flags;       /* flags specified on open */
    mode_t                 f_mode;        /* file access mode */
    loff_t                 f_pos;         /* file offset (file pointer) */
    struct fown_struct     f_owner;       /* owner data for signals */
    unsigned int           f_uid;         /* user's UID */
    unsigned int           f_gid;         /* user's GID */
    int                    f_error;       /* error code */
    struct file_ra_state   f_ra;          /* read-ahead state */
    unsigned long          f_version;     /* version number */
    void                   *f_security;   /* security module */
    void                   *private_data; /* tty driver hook */
    struct list_head       f_ep_links;    /* list of eventpoll links */
    spinlock_t             f_ep_lock;     /* eventpoll lock */
    struct address_space   *f_mapping;    /* page cache mapping */
};

#endif //_SBUNIX_VFS_VFS_H