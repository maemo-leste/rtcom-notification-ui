AC_INIT([rtcom-notification-ui], [2.1.5])
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_PATH_X
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)

PKG_PROG_PKG_CONFIG
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

PKG_CHECK_MODULES(OSSO_AF_SETTINGS, osso-af-settings)
PKG_CHECK_MODULES(NUI,
                  [hildon-1 libosso telepathy-glib dbus-glib-1 dnl
                  libhildondesktop-1 gio-unix-2.0])

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

hildondesktoplibdir="`$PKG_CONFIG --variable=hildondesktoplibdir libhildondesktop-1`"
AC_SUBST(hildondesktoplibdir)

hildonstatusmenudesktopentrydir="`$PKG_CONFIG --variable=hildonstatusmenudesktopentrydir libhildondesktop-1`"
AC_SUBST(hildonstatusmenudesktopentrydir)

AC_DEFINE_UNQUOTED([LOCALEDIR],
                   ["`pkg-config --variable=localedir osso-af-settings`"],
                   [Define the path to locales directory])

#+++++++++++++++++++
# Misc stuff
#+++++++++++++++++++

AC_ARG_ENABLE(cast-checks,  [  --disable-cast-checks   compile with GLIB cast checks disabled],[cchecks=${enableval}],cchecks=yes)
if test "x$cchecks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CAST_CHECKS"
fi

AC_ARG_ENABLE(asserts,      [  --disable-asserts       compile with GLIB assertions disabled],[asserts=${enableval}],asserts=yes)
if test "x$asserts" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_ASSERTS"
fi

AC_ARG_ENABLE(checks,       [  --disable-checks        compile with GLIB checks disabled],[checks=${enableval}],checks=yes)
if test "x$checks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CHECKS"
fi

AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], "$PACKAGE_NAME", [Default logging facility])

dnl Localization
GETTEXT_PACKAGE=rtcom-messaging-ui
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "${GETTEXT_PACKAGE}", [gettext package])

AC_OUTPUT([
	Makefile
	src/Makefile
	org.freedesktop.Telepathy.Client.NotificationUI.service
])

