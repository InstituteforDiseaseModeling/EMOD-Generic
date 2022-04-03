#!/usr/bin/python

import sys
import os
import syslog
import json
import random
import string
import math
import datetime
import time
sys.path.append( "/var/www/wsgi/" ) # this no scale
#import teaching_pages as tp

import dtk_nodedemog as nd
import dtk_genericindividual as gi
from cgi import parse_qs, escape

"""
In this app, we want to demonstrate how fertility and mortality are configured in the DTK code.
We will start with a population of 10k (?) of uniform age. We'll let the user set the fertility 
and morality params, or walk them through the options. We will run for 1 year when they hit 'submit' 
and report back the births and/or deaths.
For births, we'll record the time of each birth and render that in a monthly bar chart on its side.
For deaths, we'll do the same, but render the age (and gender?) in a monthly bar chart.
"""

human_pop = []
my_persisten_vars = {}
nursery = {} # store newborn counter by timestep (dict of int to int for now)
timestep = 0
infectiousness = []

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
    time.sleep(0.5)
    nd.set_enum_param( ( "Demographics_Filename", overlay_filename ) )
    return demog_overlay, overlay_filename

def handleConfigOverrides( config_overrides, d ):
    for param in config_overrides:
        value = d.get(param, [''])[0] 
        syslog.syslog( syslog.LOG_INFO, "Found 'posted' key & value: " + param + ", " + str( value ) )
        if value != None:
            syslog.syslog( syslog.LOG_INFO, "Setting config.json param(s): " + str( param ) + "=" + (value) )
            try:
                nd.set_param( ( param, float(value) ) )
                gi.set_param( ( param, float(value) ) )
            except Exception as ex:
                nd.set_enum_param( ( param, value ) )
                gi.set_enum_param( ( param, value ) )

def doAgeBinning( population ):
    pop_by_age_and_sex = {}
    for dec in xrange( 15 ):
        pop_by_age_and_sex[ dec ] = {}
        pop_by_age_and_sex[ dec ][ "M" ] = []
        pop_by_age_and_sex[ dec ][ "F" ] = []

    for individual in human_pop:
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
    text += "<h2>Age Decade Populations (Text)</h2>"
    text += "<table>"
    text += "<tr><td>Decade</td><td>Male</td><td>Female</td></tr>"
    for dec in pop_by_age_and_sex:
        text += "<tr><td>" + str( dec ) + "</td><td>" + str( len( pop_by_age_and_sex[ dec ][ "M" ] ) ) + "</td><td>" + str( len( pop_by_age_and_sex[ dec ][ "F" ] ) )+ "</td></tr>"
    text += "</table>"
    return text

def renderAgeBinTablePretty( pop_by_age_and_sex ): 
    text = ""
    text += "<h2>Age Decade Populations (Pretty)</h2>"
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
    text += "</table><p>"
    return text

def renderInputForm( lesson_data, d, demog_json_params, demog_overlay, page_number, use_schema = True ):
    text = "<hr><h2>Configure</h2>"
    # provide form for entering parameters
    jsony_schema = []
    if use_schema:
        schema = json.loads( gi.get_schema() )
        jsony_schema = {}
        jsony_schema.update( schema["Infection"] )
        jsony_schema.update( schema["Immunity"] )
        jsony_schema.update( schema["Individual"] )
    text += '<form action="expose_app">'
    for param in sorted(jsony_schema):
        # If we're doing page-based lessons, only display parameters from this lesson.
        # TBD: Do we want to cumulatively keep params from previous lessons?
        if lesson_data != None and param not in lesson_data["params"]:
            #syslog.syslog( syslog.LOG_INFO, "Filtering out param " + param + " coz it's not in "+ str( lesson_data["params"] ) )
            continue
        # Imperfect way to display just initial demographics params
        #if lesson_data == None  and "Sampl" in param or "Initial" in param or "Demographics" in param or "Population" in param or "Infect" in param:
        if lesson_data == None and "Incubat" not in param and "Infect" not in param and "Immun" not in param:
            continue
        my_value = jsony_schema[param]["default"]
        if param in d.keys():
            my_value = d.get(param, [''])[0] 
        text += '{0}: <input type="text" name="{1}" value="{2}"/><br><i>{3}</i><br><hr>'.format( param, param, my_value, jsony_schema[param]["description"] )

    for param in demog_json_params:
        parsed_param = param.split(".")
        if lesson_data != None and parsed_param[-1] not in lesson_data["params"]:
            continue
        my_value = 0
        if demog_overlay is not None:
            my_value = demog_overlay[ "Nodes" ][0][ parsed_param [0] ][ parsed_param[1] ]
        if param in d.keys():
            get_value = d.get(param, [''])[0] 
            my_value = get_value
        text += '{0}: <input type="text" name="{1}" value="{2}"/><br>(Demographics Parameter)<br>'.format( param, param, my_value )
        if "AgeDistributionFlag" in param:
            text += "0=Fixed, 1=Uniform, 2=Guassian, 3=Exponential, 4=Poission, 5=Log Normal, 6=Bimodal, 7=Weibull.<br>"
        text += "<hr>"
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

def renderNursery( nursery_dict ):
    text = "Babies born by timestep (30-day buckets)"
    text += "<table border=1>"
    text += "<tr><td>Month</td><td>Male</td><td>Female</td></tr>"
    if len(nursery_dict) > 0:
        print( "Nursery keys = " + str( nursery_dict ) )
        for month in xrange( 0,max(nursery_dict.keys()) ):
            print( "Rendering nursery month " + str( month ) )
            #text += "<tr><td>" + str( month ) + "</td><td>" + str( nursery_dict[month] ) + "</td><td>TBD</tr>"
            if month in nursery_dict:
                print( "Rendering nursery data for month {0}, boys={1}, girls={2}.".format( month, nursery_dict[month][0], nursery_dict[month][1] ) )
                text += "<tr><td>" + str( month ) + "</td><td>" + str( nursery_dict[month][0] ) + "</td><td>" + str( nursery_dict[month][1] ) + "</td></tr>"
            else:
                text += "<tr><td>" + str( month ) + "</td><td>0</td><td>0</td></tr>"
    else:
        text += "<tr><td>EMPTY!</td></tr>"
    text += "</table>"
    return text

def renderIntrahost( infectiousness_history ):
    text = "Infectiousness of one individual over time. (GREEN=healthy, ORANGE=infected & incubating, RED=infected & infectious)"
    text += "<table border=1><tr>"
    for inf in infectiousness_history:
        if inf == 0:
            text += "<td bgcolor='green'>" + str( inf ) + "</td>"
        elif inf == -1:
            text += "<td bgcolor='orange'>" + str( 0 ) + "</td>"
        else:
            text += "<td bgcolor='red'>" + str( inf ) + "</td>"
    text += "</tr></table>"
    return text

def my_callback( mcw, age, gender ):
    """
    This function will be renamed to create_person_callback.
    """
    global human_pop # make sure Python knows to use module-level variable human_pop
    global timestep
    global nursery
    person = {}
    person["mcw"]=mcw
    person["age"]=age/365
    person["sex"]=gender

    #syslog.syslog( syslog.LOG_INFO, "[PY] Creating human with age = {0}, gender = {1}.".format( age, gender ) )
    new_id = gi.create( ( gender, age, mcw ) ) 
    person["id"]=new_id
    human_pop.append( person )
    syslog.syslog( syslog.LOG_INFO, "Human population now = {0}.".format( len( human_pop ) ) )
    if age == 0:
        month_step = int( timestep/30.0 )
        print( "Made a baby on timestep {0}/month {1}.".format( str( timestep ), str( month_step ) ) )
        if month_step not in nursery:
            nursery[month_step] = ( 0, 0 )
        boys = nursery[month_step][0]
        girls = nursery[month_step][1]
        if gender == 0:
            boys += mcw
        else:
            girls += mcw 
        nursery[month_step] = ( boys, girls )

def conceive_baby_callback( individual_id, duration ):
    print( "{0} just got pregnant".format( individual_id ) )
    #pdb.set_trace()
    #if individual_id not in human_pop:
        #print( "Yikes! {0} is supposed to get pregnant, but she's not in our population!".format( individual_id ) )
    #else:
        #print( str( gi.is_dead( individual_id ) ) );
    gi.initiate_pregnancy( individual_id );


def update_pregnancy_callback( individual_id, dt ):
    #if individual_id not in human_pop:
        #print( "Yikes! {0} not in our population!".format( individual_id ) )
    #else:
        #print( "{0} updating pregnancy (dt = {1}).".format( individual_id, dt ) )
    return gi.update_pregnancy( individual_id, int(dt) );


def mortality_callback( age, sex, dt ):
    mortality_rate = nd.get_mortality_rate( ( age, sex, dt ) )
    return mortality_rate

def expose( action, prob ):
    #print( "expose: " + action + " with value " + str( prob ) )
    if gi.is_infected( 1 ) == False:
        print( "Johnny is not infected. Infect depending on immunity: {0}.".format( gi.get_immunity() ) )
        if random.random() < gi.get_immunity():
            #print( "Let's infect based on immunity & random draw. (Should have got your shots!)" )
            return 1
        else:
            #print( "Let's NOT infect based on immunity of {0} and random draw.".format( gi.get_immunity() ) )
            return 0
    else:
        print( "Let's NOT infect -- Johnny is already sick." )
        return 0


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
    processing_input = False
    os.chdir( "/var/www/wsgi/" )


    #
    # 1) PROCESS ALL SCRIPT INPUTS
    # 
    d = parse_qs( environ["QUERY_STRING"] )
    if len( d.keys() ) > 0:
        processing_input = True
    for param in d.keys():
        if param == "schema":
            #counter = 0
            #if "NUM_REQUESTS" in my_persisten_vars:
            #    counter = int( my_persisten_vars["NUM_REQUESTS"] )
            #    syslog.syslog( syslog.LOG_INFO, "Found NUM_REQUESTS in persistent vars with value = {0}/{1}.".format( my_persisten_vars["NUM_REQUESTS"], counter ) )
            #syslog.syslog( syslog.LOG_INFO, "Storing NUM_REQUESTS in persistent vars with value = {0}".format( counter+1 ) )
            #my_persisten_vars["NUM_REQUESTS"] = counter+1
            #output = "<pre><code>" + json.dumps( nd.get_schema(), indent=True, sort_keys=True ) + "</code></pre>"
            output = "<pre><code>" + json.dumps( json.loads( gi.get_schema() ), indent=4, sort_keys=True ) + "</code></pre>"
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

        #if param == "pagenum":
            #page_number = int( d.get(param, [''])[0] )
            #d.pop( param ) # this doesn't work
            #if page_number < len( tp.lessons ):
                #lesson_data = tp.lessons[ page_number ]
        #else:
        config_overrides.append( param )

    demog_overlay = None
    overlay_filename = None
    # With no parms, we display params from schema and wait for user to hit submit. Don't process anything.
    if processing_input: 
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
        # This is the most critical section that actually populates the human_pop array with the human population
        global human_pop # make sure Python knows to use module-level variable human_pop
        del human_pop[:]
        #human_pop = []

        syslog.syslog( syslog.LOG_INFO, "We cleared out human_pop. Should get populated via populate_from_files and callback..." )
        nd.set_callback( my_callback )
        gi.set_mortality_callback( mortality_callback )
        gi.my_set_callback( expose )
        nd.set_conceive_baby_callback( conceive_baby_callback )
        nd.set_update_preg_callback( update_pregnancy_callback )
        nd.populate_from_files()

        syslog.syslog( syslog.LOG_INFO, "Update fertility & mortality of population of size {0} for 1 year.".format( len( human_pop ) ) )
        graveyard = []
        for t in xrange(0,365): # for one year
            nd.update_fertility()
            #for hum_id in xrange( 0, len( human_pop ) ):
            global timestep
            timestep = t
            syslog.syslog( syslog.LOG_INFO, "Updating {0} individuals at timestep {1}.".format( len(human_pop), t ) )
            for human in human_pop:
                hum_id = human["id"]
                syslog.syslog( syslog.LOG_INFO, "Updating individual {0} at timestep {1}.".format( hum_id, t ) )
                gi.update( hum_id )
                serial_man = json.loads( gi.serialize() )
                print( json.dumps( serial_man["individual"], indent=4, sort_keys=True ) )
                if serial_man["individual"][ "infectiousness" ] == 0 and serial_man["individual"][ "m_is_infected" ]:
                    infectiousness.append( -1 )
                else:
                    infectiousness.append( serial_man["individual"][ "infectiousness" ] )

                #if gi.is_dead( hum_id ):
                    #syslog.syslog( syslog.LOG_INFO, "Individual {0} died.".format( hum_id ) )
                    #graveyard.append( human )
                    #try:
                        #human_pop.pop( hum_id )
                    #except Exception as ex:
                        #syslog.syslog( syslog.LOG_INFO, "Exception trying to remove individual from python list: " + str( ex ) )

                # TESTING THIS
                ipm = gi.is_possible_mother( hum_id )
                ip = gi.is_pregnant( hum_id )
                age = gi.get_age( hum_id )
                #print( "Calling cfp with {0}, {1}, {2}, and {3}.".format( str(ipm), str(ip), str(age), str(hum_id) ) )
                #nd.consider_for_pregnancy( ( ipm, ip, hum_id, age, 1.0 ) )

            print( "Done with that." )

    #
    # 4) FINALLY CREATE THE OUTPUT/RESPONSE TEXT
    # 
    # Now let's start writing html.
    output = "<html>" # json.dumps( human_pop )
    if debug or webservice_mode:
        output += json.dumps( human_pop )
    elif lesson_data != None:
        output += renderLessonText( lesson_data, page_number, environ["SCRIPT_NAME"] )
    else:
        output += "<h2>Welcome to the DTK Tutorial, Intrahost Edition. Click Run to run a simulation of 1 person who will get infected and reinfected over the course of a year.</h2><br><hr>"

    # WebApp-mode, not Service-mode -> Return HTML
    if webservice_mode == False:

        if processing_input: 
            output += "<hr><h3>Intrahost</h3>"
            output += renderIntrahost( infectiousness )

            # Age-binning for webapp viz
            pop_by_age_and_sex = doAgeBinning( human_pop )

            # Draw a stick-figure for each individual
            output += renderAgeBinTableTextOnly( pop_by_age_and_sex )
            output += renderAgeBinTablePretty( pop_by_age_and_sex )

            graveyard_by_age_and_sex = doAgeBinning( graveyard )
            #output += "<hr><h3>Graveyard</h3>"
            #output += renderAgeBinTableTextOnly( graveyard_by_age_and_sex )

            #output += "<hr><h3>Nursery</h3>"
            #output += renderNursery( nursery )


        output += renderInputForm( None, d, [], None, 0 )

    output += "</html>"

    if overlay_filename is not None:
        os.remove( overlay_filename )

    response_headers=[("Content-type","text/html"), ("Content-Length",str(len(output)))]
    start_response(status,response_headers)
    return [output]
