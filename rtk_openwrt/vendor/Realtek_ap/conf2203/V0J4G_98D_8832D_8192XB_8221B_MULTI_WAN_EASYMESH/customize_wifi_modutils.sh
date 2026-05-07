# Remove get environment partition when insert wifi module
sed -i 's/nv getenv .*/`/g' $1
