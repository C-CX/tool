#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/init.h>

#define MYFS_MAGIC 0x12345678

// 前向声明
static int myfs_create(struct mnt_idmap *, struct inode *, 
                      struct dentry *, umode_t, bool);
static int myfs_mkdir(struct mnt_idmap *, struct inode *, 
                     struct dentry *, umode_t);

// 文件操作
static const struct file_operations myfs_file_operations = {
    .open = generic_file_open,
    .read_iter = generic_file_read_iter,
    .write_iter = generic_file_write_iter,
    .llseek = generic_file_llseek,
    .fsync = generic_file_fsync,
};

// 文件inode操作
static const struct inode_operations myfs_file_inode_operations = {
    .setattr = simple_setattr,
    .getattr = simple_getattr,
};

// 目录操作
static const struct file_operations myfs_dir_operations = {
    .open = dcache_dir_open,
    .release = dcache_dir_close,
    .llseek = dcache_dir_lseek,
    .read = generic_read_dir,
    .iterate_shared = dcache_readdir,
};

// 目录inode操作
static const struct inode_operations myfs_dir_inode_operations = {
    .create = myfs_create,
    .lookup = simple_lookup,
    .mkdir = myfs_mkdir,
    .rmdir = simple_rmdir,
    .unlink = simple_unlink,
};

// 超级块操作
static const struct super_operations myfs_s_ops = {
    .statfs = simple_statfs,
    .drop_inode = generic_drop_inode,
};

// 简化的地址空间操作 - 移除有问题的函数
static const struct address_space_operations myfs_aops = {
    .write_begin = simple_write_begin,
    // 移除不存在的函数，使用默认行为
};

// 创建inode
static struct inode *myfs_make_inode(struct super_block *sb, umode_t mode)
{
    struct inode *inode = new_inode(sb);
    if (!inode)
        return NULL;
        
    inode->i_ino = get_next_ino();
    inode->i_mode = mode;
    
    // 使用新的时间设置方式
    struct timespec64 now = current_time(inode);
    inode_set_atime_to_ts(inode, now);
    inode_set_mtime_to_ts(inode, now);
    inode_set_ctime_to_ts(inode, now);
    
    inode->i_blocks = 0;
    inode->i_uid = current_fsuid();
    inode->i_gid = current_fsgid();
    
    if (S_ISREG(mode)) {
        inode->i_op = &myfs_file_inode_operations;
        inode->i_fop = &myfs_file_operations;
        inode->i_mapping->a_ops = &myfs_aops;
    } else if (S_ISDIR(mode)) {
        inode->i_op = &myfs_dir_inode_operations;
        inode->i_fop = &myfs_dir_operations;
        inc_nlink(inode);
    }
    
    return inode;
}

// 实现create函数
static int myfs_create(struct mnt_idmap *idmap, 
                      struct inode *dir, struct dentry *dentry, 
                      umode_t mode, bool excl)
{
    struct inode *inode = myfs_make_inode(dir->i_sb, mode | S_IFREG);
    if (!inode)
        return -ENOMEM;
    
    d_instantiate(dentry, inode);
    dget(dentry);
    printk(KERN_INFO "myfs: 创建文件 %s\n", dentry->d_name.name);
    return 0;
}

// 实现mkdir函数
static int myfs_mkdir(struct mnt_idmap *idmap,
                     struct inode *dir, struct dentry *dentry, umode_t mode)
{
    struct inode *inode = myfs_make_inode(dir->i_sb, mode | S_IFDIR);
    if (!inode)
        return -ENOMEM;
        
    inc_nlink(dir);  // 父目录链接计数+1
    d_instantiate(dentry, inode);
    dget(dentry);
    printk(KERN_INFO "myfs: 创建目录 %s\n", dentry->d_name.name);
    return 0;
}

// 填充超级块
static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
    printk(KERN_INFO "myfs: myfs_fill_super 挂载填充超级块\n");
    struct inode *root_inode;
    struct dentry *root_dentry;
    
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYFS_MAGIC;
    sb->s_op = &myfs_s_ops;
    sb->s_time_gran = 1;
    
    root_inode = myfs_make_inode(sb, S_IFDIR | 0755);
    if (!root_inode)
        return -ENOMEM;
    
    root_dentry = d_make_root(root_inode);
    if (!root_dentry) {
        iput(root_inode);
        return -ENOMEM;
    }
    
    sb->s_root = root_dentry;
    printk(KERN_INFO "myfs: myfs_fill_super 挂载点：%s\n",root_dentry->d_name.name);
    return 0;
}

// 挂载函数
static struct dentry *myfs_mount(struct file_system_type *fs_type,
                                 int flags, const char *dev_name, void *data)
{
    return mount_nodev(fs_type, flags, data, myfs_fill_super);
}

// 文件系统类型
static struct file_system_type myfs_type = {
    .name = "myfs",
    .mount = myfs_mount,
    .kill_sb = kill_litter_super,
    .owner = THIS_MODULE,
};

// 模块初始化
static int __init myfs_init(void)
{
    int ret = register_filesystem(&myfs_type);
    if (ret) {
        printk(KERN_ERR "myfs: 注册失败\n");
        return ret;
    }
    printk(KERN_INFO "myfs: 文件系统已注册\n");
    return 0;
}

// 模块清理
static void __exit myfs_exit(void)
{
    unregister_filesystem(&myfs_type);
    printk(KERN_INFO "myfs: 文件系统已注销\n");
}

module_init(myfs_init);
module_exit(myfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple filesystem example");