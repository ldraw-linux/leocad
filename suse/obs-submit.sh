#!/bin/bash 
PRJ="$1"
PKG=leocad

if [[ -z $1 ]] ; then
	echo "Usage: $0 OBS_PROJECT_NAME [build|force]" >&2
	exit 1
fi


BUILD=false
FORCE=false
[[ x$2 == xforce ]] && FORCE=true
[[ x$2 == xbuild ]] && {
	BUILD=true
	FORCE=true
}


if git status --porcelain |grep -q M; then
	echo "Uncommited changes:"
	git status --short
	if git status --porcelain .. |grep -q M && ! $FORCE; then
		echo "Please commit your changes to git first. To override, run:" >&2
		echo "$0 $1 force" >&2
		exit 1
	fi
fi

TAG=`git tag --points-at HEAD`
if [[ -z $TAG ]]; then 
	echo "Not on a tagged commit."
	TAG=`git describe --abbrev=0`
	[[ -z $TAG ]] && exit 1

	if ! $FORCE; then
		echo "To use the most recent tag ($TAG) as the version string, run:" >&2
		echo "$0 $1 force" >&2
		exit 1
	fi
fi
echo "Using $TAG as version string."	


D=`mktemp -d`

pushd $D
if ! osc co "$PRJ" "$PKG"; then
	popd
	rm -rf $D
	exit 1
fi

PRJD="$D/$PRJ/$PKG"

if ! cd $PRJD; then 
	popd
	rm -rf $D
	exit 1
fi
rm -rf *

popd

SPEC=$PKG.spec
sed "s/__VERSION__/$TAG/" $SPEC >$PRJD/$SPEC

pushd ..
git archive --prefix=$PKG/ HEAD | bzip2 > $PRJD/$PKG.tar.bz2
popd
HASH=`git log -1 --pretty="format:%H"`



pushd $PRJD
if $BUILD; then
	osc build
	echo "Keeping build source directory: $D"
else
	osc addremove
	osc vc -m "See the GIT history at https://github.com/ldraw-linux/$PKG/commits/$HASH"
	osc add $PKG.changes
	osc commit -m "git commit: $HASH"
	popd
	rm -rf $D
fi
