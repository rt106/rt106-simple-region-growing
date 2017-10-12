# Rt 106 algorithm wrapper for rt106-simple-region-growing

import os, glob, uuid, time, logging, subprocess

# function: run_algorithm() -- Python function for marshalling data and running algorithm.
# parameters:
#   datastore: object to be used to download and upload data
#   context:  A JSON structure that with the inputs and parameters for the algorithm.
def run_algorithm(datastore,context):

    # cleanup the input and output directories
    for f in glob.glob('/rt106/input/*') + glob.glob('/rt106/output/*'):
        os.remove(f)

    # 1.    Stage input bulk data from the datastore.
    input_path = context['inputSeries']
    response_retrieve = datastore.retrieve_series(input_path,'/rt106/input')

    # 2.    Execute algorithm code
    try:
        io_args = '/rt106/input /rt106/output'
        algorithmArgs = '%s %s %s' % (context['seedPoint'][0],
                                       context['seedPoint'][1],
                                       context['seedPoint'][2])
        subprocess.check_call('/rt106/bin/SimpleRegionGrowing %s %s' % (io_args, algorithmArgs), shell=True)
    except subprocess.CalledProcessError, e:
        logging.error('%d - %s' % (e.returncode, e.cmd))
        status = "EXECUTION_FINISHED_ERROR"
        result_context = {}
        return { 'result' : result_context, 'status' : status }

    # 3.    Cache the success or failure of your algorithm execution
    status = "EXECUTION_FINISHED_SUCCESS"

    # 4.    Upload output bulk data
    output_path = datastore.get_upload_series_path(input_path)
    response_upload = datastore.upload_series(output_path,'/rt106/output')

    output_info = ""
    if response_retrieve == 404:
        status = "EXECUTION_ABORTED_FILE_NOT_FOUND"
    elif response_upload == 409:
        status = "EXECUTION_ABORTED_UPLOAD_PATH_CONFLICT"
    else:
        output_info = response_upload.get('path')

    # 5.    Add results to the result_context.  Here, we return the path returned by the upload operation which represents where the data was stored
    result_context = {
        "outputSeries" : output_info
    }

    # return a result message
    return { 'result' : result_context, 'status' : status }
