#	@(#)Makefile	5.8 (Berkeley) 7/20/92

SUBDIR=	XNSrouted badsect clri disklabel dmesg dump dumpfs dumplfs fastboot \
	fsck fsdb icheck ifconfig init mknod mount mount_fdesc mount_kernfs \
	mount_lofs mount_nfs mount_null mount_portal mount_umap mountd \
	ncheck newfs newlfs nfsd nfsiod nologin ping quotacheck reboot \
	restore route routed savecore scsiformat shutdown slattach startslip \
	swapon tunefs umount

.if	${MACHINE} == "tahoe"
SUBDIR+=enpload
.elif	make(clean) || make(cleandir)
SUBDIR+=enpload
.endif

.include <bsd.subdir.mk>
