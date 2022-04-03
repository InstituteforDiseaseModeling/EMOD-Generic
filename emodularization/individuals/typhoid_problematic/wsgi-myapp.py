import sys
import os
import syslog
import json
import random
import string

import dtk_typhoidindividual as ti
from cgi import parse_qs, escape

var_to_param_map = {}

def expose( action, prob ):
    print( "expose: " + action + " with value " + str( prob ) ) 
    
    if random.random() < 0.2:
        if ti.get_immunity() < 1 and ti.is_infected() == False:
            print( "Let's infect based on random draw." )
            return 1
        else:
            return 0
    else:
        print( "Let's NOT infect based on random draw." )
        return 0

ti.my_set_callback( expose )

def convert_param_name_to_var_name( param_name ):
    param_name = param_name.translate(None,string.ascii_lowercase)
    param_name = param_name.replace( '_', '' )
    param_name = param_name.lower()
    return param_name

html_header = """
    <!DOCTYPE html>
    <html>
    <head>
    <meta charset="utf-8">
    <style>
        html, body { width: 100%; height: 100%; margin: 0; padding: 0; font-family: "Georgia",serif; font-size: 1em; line-height: 2; }
        section { display: inline-block; padding: 2em 3em; box-sizing: content-box; }
        dl { display: table-row; }
        dt, dd { display: table-cell; padding: 2px 4px; font-weight: normal; border-top: 1px dotted silver; vertical-align: middle; }
        dt { padding-right: 2em; }
        dl:hover dt, dl:hover dd { background: #fefe88; }
        dd, input[type=text] { font-family: "Consolas", "Menlo", serif; font-size: 1em; color: crimson; padding: 4px; }
        input[type=text] { width: 5em; }
        input[type=submit] { width: 99%; box-sizing: border-box; padding: 6px; }
        em { color: crimson; }
        section.chart { width: 90%; }
        table { display: table; width: 100%; border-collapse: separate; border-spacing: 0; }
        thead tr td { border-bottom: 1px solid pink; }
        tfoot tr td { border-top: 1px solid pink; }
        td { display: table-cell; min-height: 3em; padding: 0; border-right: 1px solid white; border-bottom: 1px solid white; }
        td samp { display: block; width: 100%; height: 100%; font-size: 1px; }
        td:nth-child(10n) { border-right: 1px solid aqua; }
        td.SUS samp { background: cornflowerblue; }
        td.PRE samp { background: limegreen; }
        td.ACU samp { background: gold; }
        td.SUB samp { background: orange; }
        td.CHR samp { background: crimson; }
        td.DED samp { background: slategrey; }
        tr.stage samp { min-height: 16px; }
        tr.infectiousness td { height: 200px; vertical-align: bottom; }
        tr.immunity td { height: 200px; vertical-align: top; }
        tr.infectiousness td, tr.immunity td { opacity: 0.8; }
        td:hover { opacity: 1 !important; }
        td.yaxis { width: 1%; opacity: 1 !important; }
        td.yaxis > div { width: 1px; display: flex; flex-direction: column; justify-content: space-between; height: 100%; }
        .switch-field { overflow: hidden; }
        .switch-field input { display: none; }
        .switch-field label {
            float: left;
            display: inline-block;
            box-sizing: border-box;
            width: 2.8em;
            margin-right: -1px;
            background-color: whitesmoke;
            border: 1px solid slategrey;
            color: crimson;
            font-weight: bold;
            text-align: center;
            text-shadow: none;
        }
        .switch-field label:hover { cursor: pointer; }
        .switch-field input:checked + label { background-color: khaki; }
        table.legend { margin: 3em 1em; width: auto; }
        table.legend td { text-indent: 0.5em; }
        table.legend samp { font-size: 1em; min-width: 2em; }
    </style>
    </head>
    <body>
    <main>
    <article>
        <section>
"""

def getEntryFormHtml( schema, age, sex, tsteps, contagion, d ): 
    # Title section
    data_entry_html = """
        <h2>Typhoid Single-Individual Intrahost Demo</h2>
        <form action="myapp">
    """

    # Allow user to configure just the Infectiousness params (note, we're not exposing everything yet).
    for param in sorted( schema.keys() ):
        if "Infectiousness" in param:
            data_entry_html += """
            <dl>
                <dt><label>{0}</label></dt><dd><input type="text" name="{1}" value="{2}"></dd>
            </dl>""".format(str(param),
                            convert_param_name_to_var_name( str( param ) ),
                            str((schema[param]["default"], d.get(param, [""])[0])[bool(param in d)])
                            #str(schema[param]["default"])
                        )

    # Now let the user specificy some demographic information about the user to experiment on, length
    # of simulation, and amount of contagion being exposed to (last is new).
    data_entry_html += """
            <dl>
                <dt><label>Age (in yrs)</label></dt><dd><input type="text" name="age" value="{0}"></dd>
            </dl>
            <dl>
                <dt><label>Sex</label></dt>
                <dd>
                  <div class="switch-field">
                    <input type="radio" id="sex_M" name="sex" value="M" {1} />
                    <label for="sex_M">M</label>
                    <input type="radio" id="sex_F" name="sex" value="F" {2} />
                    <label for="sex_F">F</label>
                  </div>
                </dd>
            </dl>
            <dl>
                <dt><label>Timesteps</label></dt><dd><input type="text" name="tsteps" value="{3}"></dd>
            </dl>
            <dl>
                <dt><label>'Contagion Population' for exposure</label></dt><dd><input type="text" name="contagion" value="{4}"></dd>
            </dl>
            <dl>
                <dt><em>Set parameters and number of timesteps, then...</em></dt>
                <dd><input type="submit" value="Run"/></dd>
            </dl>
        </form>
        </section>
    """.format( 30 if age == '' else age, 'checked' if sex != 'F' else '', 'checked' if sex == 'F' else '', 1 if tsteps == '' else tsteps, 10000000 if contagion == '' else contagion  ) 
    return data_entry_html

def getLegendHtml():
    legend_html = """
            <table class="legend">
                <tbody>
                    <tr>
                        <td class="SUS"><samp>&nbsp;</samp></td>
                        <td>Susceptible (uninfected, regular state)</td>
                    </tr>
                    <tr>
                        <td class="PRE"><samp>&nbsp;</samp></td>
                        <td>Pre-Patent (always the first stage)</td>
                    </tr>
                    <tr>
                        <td class="ACU"><samp>&nbsp;</samp></td>
                        <td>Acute (one of two possible second stages)</td>
                    </tr>
                    <tr>
                        <td class="SUB"><samp>&nbsp;</samp></td>
                        <td>Sub-Clinical (the other possible second stage)</td>
                    </tr>
                    <tr>
                        <td class="CHR"><samp>&nbsp;</samp></td>
                        <td>Chronic (possible stage after Acute or Sub-Clinical if you don't recover or die. You never leave this)</td>
                    </tr>
                    <tr>
                        <td class="DED"><samp>&nbsp;</samp></td>
                        <td>Room temperature</td>
                    </tr>
                </tbody>
            </table>
            """
    return legend_html

def getFinalAttrsHtml( tsteps, ind_json ):
    final_attrs_html = """
        <section>
            <h3>After {0} timesteps, the individual has the following attributes:</h3>
    """.format(str( tsteps ))

    serial_man = json.loads( ind_json )
    final_attrs_html += """
        <dl>
            <dt>age</dt>
            <dd>{0}</dd>
        </dl>
        <dl>
            <dt>infected status</dt>
            <dd>{1}</dd>
        </dl>
        <dl>
            <dt>number of (historical) infections</dt>
            <dd>{2}</dd>
        </dl>
    """.format(str(serial_man["individual"]["m_age"]/365.0), str(serial_man["individual"]["m_is_infected"]), str(serial_man["individual"]["_infection_count"]))

    if serial_man["individual"]["m_is_infected"]:
        final_attrs_html += ( "<dl><dt>infectiousness</dt><dd>" + str(serial_man["individual"]["typhoid_infectiousness"]) + "</dd></dl>")
        final_attrs_html += ( "<dl><dt>prepatent_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["prepatent_timer"]) + "</dd></dl>")
        final_attrs_html += ( "<dl><dt>acute_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["acute_timer"]) + "</dd></dl>")
        final_attrs_html += ( "<dl><dt>subclinical_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["subclinical_timer"]) + "</dd></dl>")
        final_attrs_html += ( "<dl><dt>chronic_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["chronic_timer"]) + "</dd></dl>")
    final_attrs_html += "</section>"
    return final_attrs_html

def getChartHtml( results, infect_max, immune_min ):
    chart_html = """
        <section class="chart">
            <table>
                <thead>
                    <tr><td colspan="{0}">Infectiousness History<td></tr>
                </thead>
                <tbody>
                    <tr class="infectiousness">
                        <td class="yaxis"><div><dfn>{1}</dfn><dfn>0.0</dfn></div></td>""".format(len(results), infect_max)

    for data in results:
        chart_html += """
                <td class="{0}"><samp title="{3}: {1}" style="height:{4}%">&nbsp;</samp></td>""".format(*data)

    chart_html += """
            </tr>
            <tr class="stage">
                <td class="yaxis"></td>"""

    for data in results:
        chart_html += """
                <td class="{0}"><samp title="{3}: {0}">&nbsp;</samp></td>""".format(*data)

    chart_html += """
            </tr>
            <tr class="immunity">
                <td class="yaxis"><div><dfn>{0}</dfn><dfn>1.0</dfn></div></td>""".format(immune_min)

    for data in results:
        chart_html += """
                <td class="{0}"><samp title="{3}: {2}" style="height:{5}%">&nbsp;</samp></td>""".format(*data)

    chart_html += """
            </tr>
        </tbody>
        <tfoot>
            <tr><td colspan="{0}">Immunity History</td><tr>
        </tfoot>
    </table>""".format(len(results))
    return chart_html

def getFooterHtml():
    footer_html = """
        </section>
        </article>
        </main>
        </body>
        </html>
    """
    return footer_html


def application(environ,start_response):
    status='200 OK' 
    d = parse_qs( environ["QUERY_STRING"] )
    tsteps = d.get('tsteps', [''])[0]
    age = d.get('age', [''])[0]
    if age != '':
        if int(age) < 0:
            age = 0
        elif int(age) > 125:
            age = 125
    sex = d.get('sex', [''])[0]
    if sex != '' and sex != 'F':
        sex = 'M'
    contagion = d.get('contagion', [''])[0]
    get_schema = d.get('schema', [''])[0]
    tai = d.get('tai', [''])[0] 

    output = html_header

    results = []

    if get_schema == '1':
        output += "<pre>" + ti.get_schema() + "</pre>"

    else:
        os.chdir( "/usr/local/wsgi/scripts/" )
        schema = json.loads( ti.get_schema() )
        syslog.syslog( syslog.LOG_INFO, str( schema.keys() ) )

        output += getEntryFormHtml( schema, age, sex, tsteps, contagion, d ) # last param sucks

        if tsteps != '':
            # We need to run the mini-model (of an individual) for 'tsteps' timesteps
            # First, set the params per the user-config (or defaults)
            if contagion != '':
                ti.set_param( ( "contagion_population", float(contagion) ) )
            for param in schema.keys():
                syslog.syslog( syslog.LOG_INFO, param )
                if "Infectiousness" in param:
                    var_name = convert_param_name_to_var_name( str( param ) )
                    var_to_param_map[ var_name ] = param
                    syslog.syslog( syslog.LOG_INFO, var_name + ", " + str( param ) )
                    qs_val = d.get(var_name, [''])[0]
                    syslog.syslog( syslog.LOG_INFO, str( qs_val ) )
                    if qs_val != None:
                        syslog.syslog( syslog.LOG_INFO, "Setting config.json param(s): " + str( param ) + "=" + (qs_val) )
                        ti.set_param( ( param, float(qs_val) ) )
            # Now create the typhoid individual
            syslog.syslog( syslog.LOG_INFO, "Creating Typhoid individual with age " + str(age) + " and sex " + 'M' )
            ti.create( long(age), sex )

            tsteps = int( tsteps )
            syslog.syslog( syslog.LOG_INFO, "tsteps = " + str(tsteps) + " of type " + str(type(tsteps)) )
            for tstep in xrange( tsteps ):
                syslog.syslog( syslog.LOG_INFO, "Updating Typhoid individual: age = {0}, infected = {1}, immunity = {2}, tstep = {3}.".format( ti.get_age(), ti.is_infected(), ti.get_immunity(), tstep ) )
                ti.update()
                ind_json = ti.serialize()
                serial_man = json.loads( ind_json )
                state = serial_man["individual"]["state_to_report"]
                inf_ness = serial_man["individual"]["typhoid_infectiousness"]

                results.append((state, inf_ness, ti.get_immunity()))
                syslog.syslog( syslog.LOG_INFO, "Finished updating individual, state = {0}, infectiousness = {1}, immunity = {2}.".format( state, inf_ness, ti.get_immunity() ) )
            syslog.syslog( syslog.LOG_INFO, "results array has size " + str(len(results) ) )

            infect_max = max(zip(*results)[1])
            immune_min = min(zip(*results)[2])
            syslog.syslog( syslog.LOG_INFO, "infect_max = {0}, immune_min = {1}".format( infect_max, immune_min ) )
            immune_min *= 0.9

            if infect_max > 0:
                for i, (a, b, c) in enumerate(results):
                    infect_factor = int(100 * (1.0*b / infect_max))
                    immune_factor = int(100 * (c - immune_min) / (1 - immune_min))
                    results[i] = (a, b, c, i, infect_factor, immune_factor)

            ind_json = ti.serialize()
            #print( str( ind_json ) )

            output += getFinalAttrsHtml( tsteps, ind_json )

            if infect_max > 0:
                output += getChartHtml( results, infect_max, immune_min )
            else:
                output += """ 
                    <section class="chart">
                        <h3>NO INFECTIONS!</h3>
                    </section>
                """

            # Legend
            output += getLegendHtml() 
            # Footer
            output += getFooterHtml() 

    response_headers=[("Content-type","text/html"), ("Content-Length",str(len(output)))]
    start_response(status,response_headers)
    return [output]
