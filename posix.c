#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include "graph.h"
#include "os.h"
#include "util.h"

void
changedir(const char *dir)
{
	if (chdir(dir) < 0)
		err(1, "chdir %s", dir);
}

int
makedirs(struct string *path, bool parent)
{
	int ret;
	struct stat st;
	char *s, *end;

	ret = 0;
	end = path->s + path->n;
	for (s = end - parent; s > path->s; --s) {
		if (*s != '/' && *s)
			continue;
		*s = '\0';
		if (stat(path->s, &st) == 0)
			break;
		if (errno != ENOENT) {
			warn("stat %s", path->s);
			ret = -1;
			break;
		}
	}
	if (s > path->s && s < end)
		*s = '/';
	while (++s <= end - parent) {
		if (*s != '\0')
			continue;
		if (ret == 0 && mkdir(path->s, 0777) < 0 && errno != EEXIST) {
			warn("mkdir %s", path->s);
			ret = -1;
		}
		if (s < end)
			*s = '/';
	}

	return ret;
}

int64_t
querymtime(const char *name)
{
	struct stat st;

	if (stat(name, &st) < 0) {
		if (errno != ENOENT)
			err(1, "stat %s", name);
		return MTIME_MISSING;
	}
#ifdef __APPLE__
	return (int64_t)st.st_mtime * 1000000000 + st.st_mtimensec;
#else
	return (int64_t)st.st_mtim.tv_sec * 1000000000 + st.st_mtim.tv_nsec;
#endif
}