FROM debian:wheezy

# install dev dependencies (compilers, etc.)
RUN buildDeps='gcc g++ make cmake vim curl' \
    && set -x \
    && apt-get update && apt-get install -y $buildDeps --no-install-recommends

# set up the folder structure
RUN mkdir /rt106

# download ITK source into the dev image (would prefer to use ADD)
RUN cd /rt106 \
    && curl --insecure -O -L https://sourceforge.net/projects/itk/files/itk/4.8/InsightToolkit-4.8.2.tar.gz \
    && tar -zvxf InsightToolkit-4.8.2.tar.gz

# compile ITK (here, since we going to build a single executable for our algorithm below, we build ITK as static which makes our final ops image small)
RUN cd /rt106 \
    && mkdir itk_build \
    && cd itk_build \
    && cmake -Wno-dev ../InsightToolkit-4.8.2 -G "Unix Makefiles" \
       -DBUILD_SHARED_LIBS=OFF -DITK_DYNAMIC_LOADING=OFF -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release \
    && make all 

# copy the source code for this algorithm into the dev image
COPY src /rt106/SimpleRegionGrowing

# compile the algorithm (here, we compile into a single statically link executable but this is not a requirement)
RUN cd /rt106 \
    && mkdir SimpleRegionGrowing_build \
    && cd SimpleRegionGrowing_build \
    && cmake -Wno-dev ../SimpleRegionGrowing \
       -DITK_DIR=/rt106/itk_build -DCMAKE_EXE_LINKER_FLAGS="-static" -DCMAKE_FIND_LIBRARY_SUFFIXES=".a" -DCMAKE_BUILD_TYPE=Release \
    && make

# emit the artifacts from the build packaged as a tarball when Docker image is run (here, the artifact is a single executable but it could be executables, libraries, etc.)
CMD tar -C /rt106/SimpleRegionGrowing_build -cvzf - SimpleRegionGrowing
