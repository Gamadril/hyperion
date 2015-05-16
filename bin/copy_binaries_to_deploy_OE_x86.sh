#!/bin/bash

if [ "$#" -ne 2 ] || ! [ -d "$1" ] || ! [ -d "$2" ]; then
  echo "Usage: $0 <BUILD-DIR> <REPO-DIR>" >&2
  exit 1
fi

# that list depends on the build platform...
filesToInclude=("libQtNetwork.so.4" "libXt.so.6" "libICE.so.6" "libSM.so.6" "libaudio.so.2" "libQtCore.so.4" "libXau.so.6" "libpng12.so.0" "libQtGui.so.4" "libXrender.so.1")

match () {
  local e
  for e in "${@:2}"; do [[ "$1" == *"$e"* ]] && return 0; done
  return 1
}

TARGET=$(uname -m)
builddir="$1"
repodir="$2"
echo build directory = $builddir
echo repository root dirrectory = $repodir

outfile="$repodir/deploy/hyperion_$TARGET.tar.gz"
outdeps="$repodir/deploy/hyperion.deps.openelec_$TARGET.tar"
echo create $outfile

cat <<EOF > "$builddir/hyperiond.sh"
#!/bin/sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/storage/hyperion/bin /storage/hyperion/bin/hyperiond "\$@"
EOF

chmod +x "$builddir/hyperiond.sh"

cat <<EOF > "$builddir/hyperion-remote.sh"
#!/bin/sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/storage/hyperion/bin /storage/hyperion/bin/hyperion-remote "\$@"
EOF

chmod +x "$builddir/hyperion-remote.sh"

tar --create --verbose --gzip --absolute-names --show-transformed-names \
	--file "$outfile" \
	--transform "s:$builddir/bin/:hyperion/bin/:" \
	--transform "s:$repodir/effects/:hyperion/effects/:" \
	--transform "s:$repodir/config/:hyperion/config/:" \
	--transform "s:$repodir/bin/hyperion.init.sh:hyperion/init.d/hyperion.init.sh:" \
	--transform "s://:/:g" \
	"$builddir/bin/hyperiond" \
	"$builddir/bin/hyperion-remote" \
	"$builddir/bin/hyperion-v4l2" \
	"$repodir/effects/"* \
	"$repodir/bin/hyperion.init.sh" \
	"$repodir/config/hyperion.config.json" \
		

deps=$(ldd $builddir/bin/hyperiond | awk 'BEGIN{ORS=" "}"$builddir/bin/hyperiond"\
~/^\//{print $builddir/bin/hyperiond}$3~/^\//{print $3}'\
 | sed 's/,$/\n/')

rm -f "$outdeps"
for dep in $deps
do
	if match "$dep" "${filesToInclude[@]}"
	then
	    echo "Copying $dep to $outdeps"
	    tar --append --absolute-names --dereference --show-transformed-names --transform 's?.*/??g' --file "$outdeps" "$dep"
    fi
done

echo "Copying $builddir/bin/hyperiond.sh to $outdeps"
tar --append --absolute-names --dereference --show-transformed-names --transform 's?.*/??g' --file "$outdeps" "$builddir/hyperiond.sh"

echo "Copying $builddir/bin/hyperion-remote.sh to $outdeps"
tar --append --absolute-names --dereference --show-transformed-names --transform 's?.*/??g' --file "$outdeps" "$builddir/hyperion-remote.sh"

echo "Compressing $outdeps"
gzip -f $outdeps

