version=3.0.0
projects=( codeslayer
           autotoolscodeslayerplugin 
           commentscodeslayerplugin
           ctagscodeslayerplugin
           filesearchcodeslayerplugin
           launchercodeslayerplugin
           navigationcodeslayerplugin
           scratchpadcodeslayerplugin 
           wordcompletioncodeslayerplugin
          )
          
BASE_FOLDER=`pwd`;          

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

for project in ${projects[*]}
do

  cd $BASE_FOLDER

  wget http://codeslayer.googlecode.com/files/${project}-${version}.tar.gz
  
  tar -zxvf ${project}-${version}.tar.gz
  
  cd ${project}-${version}
  
  ./configure
  make
  sudo make install
  sudo ldconfig

done
