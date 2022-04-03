import sys
import os
import syslog
import json
import random
import string
import math
import datetime
import time
sys.path.append( "/usr/local/wsgi/scripts" ) # this no scale
import teaching_pages as tp

import dtk_nodedemog
from cgi import parse_qs, escape

mydata = []

def createDemographicsOverlay( demog_json_params, d ):
    demog_overlay = json.loads( open( "demographics.json" ).read() )
    for param in demog_json_params:
        keys = param.split( "." )
        #syslog.syslog( syslog.LOG_INFO, "Demog Overriding {0}, {1} with {2}/{3}.".format( keys[0], keys[1], param, demog_json_params[ param ] ) )
        value = d.get(param, [''])[0] 
        if value != None and len(value) > 0:
            demog_overlay[ "Nodes" ][0][ keys[0] ][ keys[1] ] = int(value)
    timestamp = str(datetime.datetime.now()).replace('-', '_' ).replace( ' ', '_' ).replace( ':', '_' ).replace( '.', '_' )
    overlay_filename = "demog_overlay_" + timestamp  + ".json"
    with open( overlay_filename, "w" ) as demog_overlay_file:
        demog_overlay_file.write( json.dumps( demog_overlay ) )
    dtk_nodedemog.set_enum_param( ( "Demographics_Filename", overlay_filename ) )
    return demog_overlay, overlay_filename

def handleConfigOverrides( config_overrides, d ):
    for param in config_overrides:
        value = d.get(param, [''])[0] 
        syslog.syslog( syslog.LOG_INFO, "Found 'posted' key & value: " + param + ", " + str( value ) )
        if value != None:
            syslog.syslog( syslog.LOG_INFO, "Setting config.json param(s): " + str( param ) + "=" + (value) )
            try:
                dtk_nodedemog.set_param( ( param, float(value) ) )
            except Exception as ex:
                dtk_nodedemog.set_enum_param( ( param, value ) )

def doAgeBinning( population ):
    pop_by_age_and_sex = {}
    for dec in xrange( 20 ):
        pop_by_age_and_sex[ dec ] = {}
        pop_by_age_and_sex[ dec ][ "M" ] = []
        pop_by_age_and_sex[ dec ][ "F" ] = []

    for individual in population:
        mcw = int(individual["mcw"])
        disp_height = 42 * math.log(mcw)
        age = individual["age"]
        #syslog.syslog( syslog.LOG_INFO, "age = " + str( age ) )
        age_dec = int(age/10)
        if individual["sex"] == 0: 
            pop_by_age_and_sex[ age_dec ][ "M" ].append( mcw )
        else:
            pop_by_age_and_sex[ age_dec ][ "F" ].append( mcw )
    return pop_by_age_and_sex

def renderAgeBinTableTextOnly( pop_by_age_and_sex ):
    text = ""
    text += "<table>"
    text += "<tr><td>Age Decade Populations</tr>"
    text += "<tr><td>Decade</td><td>Male</td><td>Female</td></tr>"
    for dec in pop_by_age_and_sex:
        text += "<tr><td>" + str( dec ) + "</td><td>" + str( len( pop_by_age_and_sex[ dec ][ "M" ] ) ) + "</td><td>" + str( len( pop_by_age_and_sex[ dec ][ "F" ] ) )+ "</td></tr>"
    text += "</table>"
    return text

def renderAgeBinTablePretty( pop_by_age_and_sex ): 
    text = ""
    text += "<h2>Age Decade Populations</td></h2>"
    text += "<table border=1>"
    text += "<tr><td>Decade</td><td>Male</td><td>Female</td></tr>"
    for dec in pop_by_age_and_sex:
        text += "<tr><td>" + str( dec ) + "</td><td>"
        for guy in pop_by_age_and_sex[ dec ][ "M" ]:
            disp_height = 42 * (1+math.log(guy))
            disp_height *= 0.5
            text += '<img src="http://www.clipartbest.com/cliparts/LiK/egM/LiKegMyXT.png" height="' + str(disp_height) + '"/>'
        text += "</td><td>"
        for gal in pop_by_age_and_sex[ dec ][ "F" ]:
            disp_height = 42 * (1+math.log(gal))
            disp_height *= 0.5
            text += '<img src="http://www.clker.com/cliparts/V/O/4/c/M/P/boy-and-girl-stick-figure-red-md.png" height="' + str(disp_height) + '"/>'
        text += "</td></tr>"
    text += "</table>"
    return text

def renderInputForm( lesson_data, d, demog_json_params, demog_overlay, page_number ):
    text = ""
    # provide form for entering parameters
    jsony_schema = json.loads( dtk_nodedemog.get_schema() )
    text += '<form action="nodedemogapp">'
    for param in sorted(jsony_schema):
        # If we're doing page-based lessons, only display parameters from this lesson.
        # TBD: Do we want to cumulatively keep params from previous lessons?
        if lesson_data != None and param not in lesson_data["params"]:
            #syslog.syslog( syslog.LOG_INFO, "Filtering out param " + param + " coz it's not in "+ str( lesson_data["params"] ) )
            continue
        # Imperfect way to display just initial demographics params
        if lesson_data == None  and "Sampl" not in param and "Initial" not in param and "Demographics" not in param and "Population" not in param:
            continue
        my_value = jsony_schema[param]["default"]
        if param in d.keys():
            my_value = d.get(param, [''])[0] 
        text += '{0}: <input type="text" name="{1}" value="{2}"/><br><i>{3}</i><hr>'.format( param, param, my_value, jsony_schema[param]["description"] )

    for param in demog_json_params:
        parsed_param = param.split(".")
        if lesson_data != None and parsed_param[-1] not in lesson_data["params"]:
            continue
        my_value = demog_overlay[ "Nodes" ][0][ parsed_param [0] ][ parsed_param[1] ]
        if param in d.keys():
            get_value = d.get(param, [''])[0] 
            my_value = get_value
        text += '{0}: <input type="text" name="{1}" value="{2}"/><br>(Demographics Parameter)<hr>'.format( param, param, my_value )
        if param=="Age_Initialization_Distribution_Type":
            text += "0=Fixed, 1=Uniform, 2=Guassian, 3=Exponential, 4=Poission, 5=Log Normal, 6=Bimodal, 7=Weibull.<br>"
    text += '<input type="hidden" name="pagenum" value="{0}"/>'.format( page_number )

    text += '<input type="submit" value="Run"/>'
    text += "</form>"
    return text

def renderLessonText( lesson_data, pagenum, scriptname ):
    text = ""
    text += "<h1>" + lesson_data[ "title" ] + "</h2>"
    text += "<h2>Section: " + lesson_data[ "section" ] + "</h2><hr>"
    text += "<h3>Page: " + lesson_data[ "page" ] + "</h3><hr>"
    text += "<h3>Topic: " + lesson_data[ "topic" ] + "</h3><hr>"
    text += "<i>" + lesson_data[ "blurb" ] + "</i><hr>"
    text += "<a href='" + scriptname + "?pagenum={0}'>Next</a><hr>".format( pagenum+1 )
    return text

def create_person_callback( mcw, age, gender ):
    global mydata
    person = {}
    person["mcw"]=mcw
    person["age"]=age/365
    person["sex"]=gender
    mydata.append( person )


def application(environ,start_response):

    #
    # 0) INITIALIZE VARIABLES
    # 
    status='200 OK' 
    debug = False
    webservice_mode = False
    lesson_data = None
    page_number = 0
    demog_json_params = [ "NodeAttributes.InitialPopulation", "IndividualAttributes.AgeDistributionFlag", "IndividualAttributes.AgeDistribution1", "IndividualAttributes.AgeDistribution2" ]
    config_overrides = []
    os.chdir( "/usr/local/wsgi/scripts/" )

    #
    # 1) PROCESS ALL SCRIPT INPUTS
    # 
    d = parse_qs( environ["QUERY_STRING"] )
    for param in d.keys():
        if param == "schema":
            #output = "<pre><code>" + json.dumps( dtk_nodedemog.get_schema(), indent=True, sort_keys=True ) + "</code></pre>"
            output = "<pre><code>" + json.dumps( json.loads( dtk_nodedemog.get_schema() ), indent=4, sort_keys=True ) + "</code></pre>"
            response_headers=[("Content-type","text/html"), ("Content-Length",str(len(output)))]
            start_response(status,response_headers)
            return [output]

        if param == "raw":
            debug = True
            continue

        if param == "rawonly":
            webservice_mode = True
            continue

        if param in demog_json_params:
            continue # for now

        if param == "pagenum":
            page_number = int( d.get(param, [''])[0] )
            d.pop( param ) # this doesn't work
            lesson_data = tp.lessons[ page_number ]
        else:
            config_overrides.append( param )

    #
    # 2) PROCESS DTK PARAMETER INPUTS (config.json and demographics.json)
    # 
    # Create demographics_overlay.json from demog_json_params, write to demog_over_<tstamp>.json, and add to Demographics_Filenames.json
    demog_overlay, overlay_filename = createDemographicsOverlay( demog_json_params, d )

    # Override default config.json params with those set by webapp user
    handleConfigOverrides( config_overrides, d )

    #
    # 3) RUN THE MODEL WITH THE USER'S PARAMETERS
    # 
    # This is the most critical section that actually populates the mydata array with the human population
    global mydata
    del mydata
    mydata = []

    dtk_nodedemog.set_callback( create_person_callback )
    dtk_nodedemog.populate_from_files()

    #
    # 4) FINALLY CREATE THE OUTPUT/RESPONSE TEXT
    # 
    # Now let's start writing html.
    output = "<html>" # json.dumps( mydata )
    if debug or webservice_mode:
        output += json.dumps( mydata )
    elif lesson_data != None:
        output += renderLessonText( lesson_data, page_number, environ["SCRIPT_NAME"] )

    # WebApp-mode, not Service-mode -> Return HTML
    if webservice_mode == False:
        output += renderInputForm( lesson_data, d, demog_json_params, demog_overlay, page_number )

        # Age-binning for webapp viz
        pop_by_age_and_sex = doAgeBinning( mydata )

        # Draw a stick-figure for each individual
        output += renderAgeBinTableTextOnly( pop_by_age_and_sex )
        output += renderAgeBinTablePretty( pop_by_age_and_sex )

    output += "</html>"

    os.remove( overlay_filename )

    response_headers=[("Content-type","text/html"), ("Content-Length",str(len(output)))]
    start_response(status,response_headers)
    return [output]
