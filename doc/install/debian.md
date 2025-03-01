## Debian/Ubuntu install
The following command will install tools and libraries required to build and install the Apache Thrift compiler and C++ libraries on a Debian/Ubuntu Linux based system.

	sudo apt-get install automake bison flex g++ git libboost-all-dev libevent-dev libssl-dev libtool make pkg-config

Debian 7/Ubuntu 12 users need to manually install a more recent version of automake and (for C++ library and test support) boost:

    wget http://ftp.debian.org/debian/pool/main/a/automake-1.15/automake_1.15-3_all.deb
    sudo dpkg -i automake_1.15-3_all.deb

    wget http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz                                                                      tar xvf boost_1_60_0.tar.gz
    cd boost_1_60_0
    ./bootstrap.sh
    sudo ./b2 install

## Optional packages

If you would like to build Apache Thrift libraries for other programming languages you may need to install additional packages. The following languages require the specified additional packages:

 * Java
	* packages: gradle (version 6.9.2)
	* You will also need Java JDK v1.8 or higher. Type **javac** to see a list of available packages, pick the one you prefer and **apt-get install** it (e.g. default-jdk).
 * Ruby
	* ruby-full ruby-dev ruby-rspec rake rubygems bundler
 * Python
	* python-all python-all-dev python-all-dbg
 * Perl
	* libbit-vector-perl libclass-accessor-class-perl
 * Php, install
	* php5-dev php5-cli phpunit
 * C_glib
	* libglib2.0-dev
 * Erlang
	* erlang-base erlang-eunit erlang-dev rebar
 * NetStd
	* apt-transport-https dotnet-sdk-6.0 aspnetcore-runtime-6.0
 * Thrift Compiler for Windows
	* mingw-w64 mingw-w64-x86-64-dev nsis
 * Rust
	* rustc cargo
 * Haxe
	* haxe
 * Lua
    * lua5.3 liblua5.3-dev
 * NodeJs
    * nodejs npm
 * dotnetcore
    * https://www.microsoft.com/net/learn/get-started/linuxubuntu
 * d-lang
    * curl -fsS https://dlang.org/install.sh | bash -s dmd
 * dart & pub
    * https://www.dartlang.org/install/linux
    * https://www.dartlang.org/tools/pub/installing


## Additional reading

For more information on the requirements see: [Apache Thrift Requirements](/docs/install)

For more information on building and installing Thrift see: [Building from source](/docs/BuildingFromSource)
