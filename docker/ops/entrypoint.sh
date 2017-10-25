#!/bin/sh
# Copyright (c) General Electric Company, 2017.  All rights reserved.

# start a server to serve the REST API to the algorithm container
/usr/bin/python ./rt106GenericAdaptorREST.py & sleep 3

# start a server to serve the execution requests
if test ${DATASTORE_URI:-undefined} = 'undefined'; then
  datastore='http://datastore:5106'
else
  datastore=${DATASTORE_URI}
fi
/usr/bin/python ./rt106GenericAdaptorAMQP.py --broker rabbitmq --dicom $datastore
