#!/bin/bash
# Forked from https://github/InspectorWidget/InspectorWidgetCollector cmake/osxbundle/fixup_bundle.sh

if [ "$(uname)" != "Darwin" ]; then
	echo "Not fixing bundle (only for macOS)"
	exit 0
fi

if [ "$#" != 2 ]; then
	echo "usage: $0 /path/to/install/root relative/lib/destination"
	exit 1
fi

cd "$1"

function buildlist() {
	otool -L "$@" | 
		grep -E '^\s+/' |
		grep -vE '^\s+/System/' |
		grep -vE '^\s+/usr/lib/' |
		perl -pe 's|^\s+(/.*)\s\(.*$|$1|' |
		grep -vE ":$" |
		sort -u
}
export -f buildlist

DEST="$2"
LDEST="@rpath"
TARGETS="$(find . \( -perm +111 -and -type f \))"
FOUNDLIBS="$(buildlist $TARGETS)"
PFOUNDLIBS=""

echo "Fixing targets: $TARGETS"
echo "Found libs to bundle: $FOUNDLIBS"

while [ "$FOUNDLIBS" != "$PFOUNDLIBS" ]; do
	PFOUNDLIBS="$FOUNDLIBS"
	FOUNDLIBS="$(buildlist $TARGETS $PFOUNDLIBS)"
done

echo "Added libs to bundle: $FOUNDLIBS"

INTOOL_CALL=()

for lib in $FOUNDLIBS; do
	libname="$(basename "$lib")"

	INTOOL_CALL+=(-change "$lib" "$LDEST/$libname")
	cp "$lib" "$DEST/$libname"
	chmod 644 "$DEST/$libname"

	echo "Fixing up dependency: $libname"
done

for lib in $FOUNDLIBS; do
	libname="$(basename "$lib")"
	lib="$DEST/$libname"

	install_name_tool ${INTOOL_CALL[@]} -id "$LDEST/$libname" "$lib"
done

for target in $TARGETS; do
	install_name_tool ${INTOOL_CALL[@]} "$target"
done

