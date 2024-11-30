hildondesktoplib_LTLIBRARIES = librtcom-notification-ui.la

librtcom_notification_ui_la_CFLAGS = \
			-Wall -Werror $(NUI_CFLAGS)

librtcom_notification_ui_la_LDFLAGS = \
			-Wl,--as-needed $(NUI_LIBS) -module
			-avoid-version -Wl, no-undefined

librtcom_notification_ui_la_SOURCES = \
			$(OFONO_GDBUS_WRAPPERS) \
			nui-status-plugin.c \
			nui-call-monitor.c

OFONO_GDBUS_WRAPPERS = \
			org.ofono.Manager.c \
			org.ofono.Modem.c \
			org.ofono.VoiceCallManager.c \
			org.ofono.VoiceCall.c

BUILT_SOURCES = nui-marshal.c nui-marshal.h \
		$(OFONO_GDBUS_WRAPPERS) $(OFONO_GDBUS_WRAPPERS:.c=.h)

nui-marshal.c: nui-marshal.list
	$(GLIB_GENMARSHAL) --prefix=nui $< --body --internal > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ ) && rm -f xgen-$(@F)

nui-marshal.h: nui-marshal.list
	$(GLIB_GENMARSHAL) --prefix=nui $< --header --internal > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ ) && rm -f xgen-$(@F)

.NOTPARALLEL:
%.c: %.xml
	gdbus-codegen --c-namespace Nui --interface-prefix org. \
		      --generate-c-code $(@:%.c=%) $<

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in
