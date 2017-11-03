# Copyright (c) General Electric Company, 2017.  All rights reserved.

FROM rt106/rt106-algorithm-sdk

# add the artifacts emitted from the dev container
ADD simpleRegionGrowing.tar.gz /rt106/bin

# add the adaptor code specialized for this algorithm
ADD rt106SpecificAdaptorCode.py rt106SpecificAdaptorDefinitions.json /rt106/

# set permissions
USER root
RUN chown -R rt106:rt106 /rt106

# set the working directory
WORKDIR /rt106

# establish user (created in the base image)
USER rt106:rt106

# configure the default port for an analytic, can be overridden in entrypoint
EXPOSE 7106

# entry point
CMD ["/rt106/entrypoint.sh"]
