#	@(#)Makefile	5.7 (Berkeley) 7/20/92

# BROKEN pstat
SUBDIR=	ac accton amd arp chown chroot config cron dev_mkdb diskpart \
	edquota gettable htable implog implogd inetd iostat kgmon kvm_mkdb \
	lpr mkproto mtree named portmap pwd_mkdb quot quotaon repquota \
	rmt rwhod sa sendmail sliplogin syslogd traceroute trpt trsp \
	update vipw

.if   make(clean) || make(cleandir)
SUBDIR+=arff bad144 dlmpcc flcopy rxformat timed
.elif ${MACHINE} == "hp300"
SUBDIR+=timed
.elif ${MACHINE} == "tahoe"
SUBDIR+=dlmpcc timed
.elif ${MACHINE} == "vax"
SUBDIR+=arff bad144 flcopy rxformat timed
.elif ${MACHINE} == "i386"
SUBDIR+=bad144 
.endif

.include <bsd.subdir.mk>
