Thrift Java Software Library

License
=======

Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements. See the NOTICE file
distributed with this work for additional information
regarding copyright ownership. The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License. You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the
specific language governing permissions and limitations
under the License.

Building and installing from source
===================================

When using a CMake build from the source distribution on Linux the
easiest way to build and install is this simple command line:

    make all && sudo make install/fast

It is important to use the install/fast option to eliminate
the automatic rebuild by dependency that causes issues because
the build tooling is designed to work with cached files in the
user home directory during the build process. Instead this builds
the code in the expected local build tree and then uses CMake
install code to copy to the target destination.

Building Thrift with Gradle without CMake/Autoconf
==================================================

The Thrift Java source is not build using the GNU tools, but rather uses
the Gradle build system, which tends to be predominant amongst Java
developers.

Currently we use gradle 6.9.2 to build the Thrift Java source. The usual way to setup gradle
project is to include the gradle-wrapper.jar in the project and then run the gradle wrapper to
bootstrap setting up gradle binaries. However to avoid putting binary files into the source tree we
have ignored the gradle wrapper files. You are expected to install it manually, as described in
the [gradle documentation](https://docs.gradle.org/current/userguide/installation.html), or
following this step (which is also done in the travis CI docker images):

```bash
export GRADLE_VERSION="6.9.2"
# install dependencies
apt-get install -y --no-install-recommends openjdk-11-jdk-headless wget unzip
# download gradle distribution
wget https://services.gradle.org/distributions/gradle-$GRADLE_VERSION-bin.zip -q -O /tmp/gradle-$GRADLE_VERSION-bin.zip
# check binary integrity
echo "8b356fd8702d5ffa2e066ed0be45a023a779bba4dd1a68fd11bc2a6bdc981e8f  /tmp/gradle-$GRADLE_VERSION-bin.zip" | sha256sum -c -
# unzip and install
unzip -d /tmp /tmp/gradle-$GRADLE_VERSION-bin.zip
mv /tmp/gradle-$GRADLE_VERSION /usr/local/gradle
ln -s /usr/local/gradle/bin/gradle /usr/local/bin
```

After the above step, `gradle` binary will be available in `/usr/local/bin/`. You can further choose
to locally create the gradle wrapper (even if they are ignored) using:

```bash
gradle wrapper --gradle-version $GRADLE_VERSION
```

To compile the Java Thrift libraries, simply do the following:

```bash
gradle
```

Yep, that's easy. Look for `libthrift-<version>.jar` in the build/libs directory.

The default build will run the unit tests which expect a usable
Thrift compiler to exist on the system. You have two choices for
that.

* Build the Thrift executable from source at the default
  location in the source tree. The project is configured
  to look for it there.
* Install the published binary distribution to have Thrift
  executable in a known location and add the path to the
  ~/.gradle/gradle.properties file using the property name
  "thrift.compiler". For example this would set the path in
  a Windows box if Thrift was installed under C:\Thrift

    thrift.compiler=C:/Thrift/thrift.exe

To just build the library without running unit tests you simply do this.

```bash
gradle assemble
```

To install the library in the local Maven repository location
where other Maven or Gradle builds can reference it simply do this.

```bash
gradle install
```

The library will be placed in your home directory under .m2/repository

To include Thrift in your applications simply add libthrift.jar to your
classpath, or install if in your default system classpath of choice.


Build Thrift behind a proxy:


```bash
gradle -Dhttp.proxyHost=myproxyhost -Dhttp.proxyPort=8080 -Dhttp.proxyUser=thriftuser -Dhttp.proxyPassword=topsecret
```

or via

```bash
./configure --with-java GRADLE_OPTS='-Dhttp.proxyHost=myproxyhost -Dhttp.proxyPort=8080 -Dhttp.proxyUser=thriftuser -Dhttp.proxyPassword=topsecret'
```

Unit Test HTML Reports
======================

The build will automatically generate an HTML Unit Test report. This can be found
under build/reports/tests/test/index.html. It can be viewed with a browser
directly from that location.


Clover Code Coverage for Thrift
===============================

The build will optionally generate Clover Code coverage if the Gradle property
`cloverEnabled=true` is set in ~/.gradle/gradle.properties or on the command line
via `-PcloverEnabled=true`. The generated report can be found under the location
build/reports/clover/html/index.html. It can be viewed with a browser
directly from that location. Additionally, a PDF report is generated and is found
under the location build/reports/clover/clover.pdf.

The following command will build, unit test, and generate Clover reports:

```bash
gradle -PcloverEnabled=true
```

Publishing Maven Artifacts to Maven Central
===========================================

The Automake build generates a Makefile that provides the correct parameters
when you run the build provided the configure.ac has been set with the correct
version number. The Gradle build will receive the correct value for the build.
The same applies to the CMake build, the value from the configure.ac file will
be used if you execute these commands:

```bash
make maven-publish   -- This is for an Automake Linux build
make MavenPublish    -- This is for a CMake generated build
```

The uploadArchives task in Gradle is preconfigured with all necessary details
to sign and publish the artifacts from the build to the Apache Maven staging
repository. The task requires the following externally provided properties to
authenticate to the repository and sign the artifacts. The preferred approach
is to create or edit the ~/.gradle/gradle.properties file and add the following
properties to it.

```properties
# Signing key information for artifacts PGP signature (values are examples)
signing.keyId=24875D73
signing.password=secret
signing.secretKeyRingFile=/Users/me/.gnupg/secring.gpg

# Apache Maven staging repository user credentials
mavenUser=meMyselfAndI
mavenPassword=MySuperAwesomeSecretPassword
```

NOTE: If you do not have a secring.gpg file, see the
[gradle signing docs](https://docs.gradle.org/current/userguide/signing_plugin.html)
for instructions on how to generate it.

It is also possible to manually publish using the Gradle build directly.
With the key information and credentials in place the following will generate
if needed the build artifacts and proceed to publish the results.

```bash
gradle -Prelease=true uploadArchives
```

It is also possible to override the target repository for the Maven Publication
by using a Gradle property, for example you can publish signed JAR files to your
company internal server if you add this to the command line or in the
~/.gradle/gradle.properties file. The URL below assumes a Nexus Repository.

```properties
maven-repository-url=https://my.company.com/service/local/staging/deploy/maven2
```

Or the same on the command line:

```bash
gradle -Pmaven-repository-url=https://my.company.com/service/local/staging/deploy/maven2 -Prelease=true -Pthrift.version=0.11.0 uploadArchives
```


Dependencies
============

Gradle
http://gradle.org/

# Breaking Changes

## 0.13.0

* The signature of the 'process' method in TAsyncProcessor and TProcessor has
changed to remove the boolean return type and instead rely on Exceptions.

* Per THRIFT-4805, TSaslTransportException has been removed. The same condition
is now covered by TTansportException, where `TTransportException.getType() == END_OF_FILE`.

## 0.12.0

The access modifier of the AutoExpandingBuffer class has been changed from
public to default (package) and will no longer be accessible by third-party
libraries.

The access modifier of the ShortStack class has been changed from
public to default (package) and will no longer be accessible by third-party
libraries.

