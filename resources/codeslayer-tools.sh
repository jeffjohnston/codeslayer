projects=( codeslayer 
           codeslayer.spike 
           codeslayer.autotools-plugin 
           codeslayer.comments-plugin
           codeslayer.ctags-plugin
           codeslayer.filesearch-plugin
           codeslayer.launcher-plugin
           codeslayer.navigation-plugin
           codeslayer.scratchpad-plugin 
           codeslayer.snippets-plugin 
           codeslayer.wordcomplete-plugin
          )

execute_make_all () 
{  
  for project in ${projects[*]}
  do
    echo "building ${project}..."
    
    case $1 in 
    -dev)
      export PKG_CONFIG_PATH=$HOME/install/lib/pkgconfig

      cd $BASE_FOLDER/$project
      autoreconf

      BUILD_FOLDER=$BASE_FOLDER/$project/build
      
      if [ ! -e "$BUILD_FOLDER" ]
      then
        mkdir $BUILD_FOLDER
      fi
      
      cd $BUILD_FOLDER
      rm -Rf $BUILD_FOLDER/*
      
      if [ $project == "codeslayer" && "$2" == "-docs" ]
      then
        LD_LIBRARY_PATH=$BASE_FOLDER/codeslayer/build/codeslayer/.libs/
        export LD_LIBRARY_PATH

        $BASE_FOLDER/$project/configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install --enable-codeslayer-dev=true --enable-gtk-doc 
      else
        $BASE_FOLDER/$project/configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install --enable-codeslayer-dev=true
      fi

      make
      make dist
      make install
      ;;
    -prod)
      export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
      
      cd $BASE_FOLDER/$project
      autoreconf

      BUILD_FOLDER=$BASE_FOLDER/$project/build-prod
      
      if [ ! -e "$BUILD_FOLDER" ]
      then
        mkdir $BUILD_FOLDER
      fi
      
      cd $BUILD_FOLDER
      rm -Rf $BUILD_FOLDER/*
      
      if [ $project == "codeslayer" && "$2" == "-docs" ]
      then
        LD_LIBRARY_PATH=$BASE_FOLDER/codeslayer/build-prod/codeslayer/.libs/
        export LD_LIBRARY_PATH

        $BASE_FOLDER/$project/configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --enable-gtk-doc
      else
        $BASE_FOLDER/$project/configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0"
      fi
      
      make
      make dist
      sudo make install
      ;;
    *)
      echo "invalid make-install option...use '-dev' or '-prod'."
      ;;  
    esac
    
  done
}

execute_git_clone () 
{

  for project in ${projects[*]}
  do
    echo "clone ${project}..."
    
    git clone https://code.google.com/p/${project}/
    
  done
}

execute_git_status () 
{

  for project in ${projects[*]}
  do
    echo "status ${project}..."
    
    cd $BASE_FOLDER/$project
    
    git status
    
  done
}

execute_git_commit () 
{

  for project in ${projects[*]}
  do
    echo "commit ${project}...$1"
    
    if [ "$1" == "" ]
    then
      echo "you need to provide a message"    
    else
      cd $BASE_FOLDER/$project
      git add
      git commit -a -m $1
      git push
    fi
  done
}

BASE_FOLDER=`pwd`;

case $1 in 
make-all)
  execute_make_all $2 $3
  ;;
git-clone)
  execute_git_clone
  ;;
git-status)
  execute_git_status
  ;;
git-commit)
  execute_git_commit $2
  ;;
*)
  echo "invalid command"
  ;;  
esac
