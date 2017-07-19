#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

typedef (*orig_send)(int, const void *, size_t, int);

ssize_t send(int fd, const void *buf, size_t len, int flags) {
	printf("send: %s\n", (char *)buf);
	orig_send o_s;
	o_s = (orig_send)dlsym(RTLD_NEXT, "send");
	return o_s(fd, buf, len, flags);

}
