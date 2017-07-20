#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");

static void **syscall_table = (void *)__SYSCALL_TABLE_ADDRESS__;

asmlinkage ssize_t (*org_write)(int fd, const void *buf, size_t len);
asmlinkage static int hook_write(int fd, const void *buf, size_t len) {
	printk(KERN_INFO "[*] hook_write(\"%s\" %d)\n", (char *)buf, len);
	return org_write(fd, buf, len);
}

static void save_original_syscall_address(void)
{
    org_write = syscall_table[__NR_write];
}

static void change_page_attr_to_rw(pte_t *pte)
{
    set_pte_atomic(pte, pte_mkwrite(*pte));
}

static void change_page_attr_to_ro(pte_t *pte)
{
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

static void replace_system_call(void *new)
{
    unsigned int level = 0;
    pte_t *pte;

    pte = lookup_address((unsigned long) syscall_table, &level);
    /* Need to set r/w to a page which syscall_table is in. */
    change_page_attr_to_rw(pte);

    syscall_table[__NR_write] = new;
    /* set back to read only */
    change_page_attr_to_ro(pte);
}

static int hook_init(void) {
	printk(KERN_INFO "[*] hook_init\n");
	printk(KERN_INFO "[*] syscall_table_address is 0x%p\n", syscall_table);

	save_original_syscall_address();
	printk(KERN_INFO "[*] orogial sys_write address is 0x%p\n", org_write);

	replace_system_call(hook_write);

	printk(KERN_INFO "[*] system call replaced\n");

	return 0;

}

static void hook_exit(void) {
	printk(KERN_INFO "[*] hook_exit\n");
	syscall_table[__NR_write] = org_write;
}

module_init(hook_init);
module_exit(hook_exit);
