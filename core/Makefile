SUBDIR = xnet base dispatcher conn proto sidWatcher writer

all-recursive clean-recursive depend-recursive install-recursive release-recursive:
	@target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIR)'; \
	for subdir in $$list; do \
		echo "===> Making $$target in $$subdir..."; \
		(cd $$subdir && $(MAKE) $$target) \
	done;
 
all:	all-recursive

clean:	clean-recursive

depend: depend-recursive

release: release-recursive

install: install-recursive copy-file

	
.PHONY : base xnet conn dispatcher proto writer
xnet:
	make "SUBDIR = xnet"
base:
	make "SUBDIR = base"
