#!/usr/bin/python

#import pymongo
import argparse
import matplotlib.pyplot as plt
import numpy as np
import json
import sys
import os
import pylab
from math import sqrt, ceil

#mc = pymongo.Connection("ingolstadt").test.things

def plotOneFromDisk():
    with open( sys.argv[1] ) as ref_sim:
        ref_data = json.loads( ref_sim.read() )

    num_chans = ref_data["Header"]["Channels"]
    idx = 0
    for chan_title in sorted(ref_data["Channels"]):
        try:
            subplot = plt.subplot( 4,5, idx  ) 
            subplot.plot( ref_data["Channels"][chan_title]["Data"], 'r-' )
            plt.title( chan_title )
        except Exception as ex:
            print( str(ex) + ", idx = " + str(idx) )
        if idx == 4*5:
            break

    plt.show()

def plotCompareFromDisk( reference, comparison, label = "" ):
    with open( reference ) as ref_sim:
        ref_data = json.loads( ref_sim.read() )

    with open( comparison ) as test_sim:
        test_data = json.loads( test_sim.read() )

    num_chans = ref_data["Header"]["Channels"]

    square_root = ceil(sqrt(num_chans))

    n_figures_x = square_root
    n_figures_y = ceil(float(num_chans)/float(square_root)) #Explicitly perform a float division here as integer division floors in Python 2.x

    if label == "unspecified":
        label = sys.argv[1]
    figure = plt.figure(label.split('/')[-1])   # label includes the full (relative) path to the scenario, take just the final directory
    F = pylab.gcf()
    DefaultSize = F.get_size_inches()

    ref_tstep = 1
    if( "Simulation_Timestep" in ref_data["Header"] ):
        ref_tstep = ref_data["Header"]["Simulation_Timestep"]

    tst_tstep = 1
    if( "Simulation_Timestep" in test_data["Header"] ):
        tst_tstep = test_data["Header"]["Simulation_Timestep"]

    idx = 1
    for chan_title in sorted(ref_data["Channels"]):
        if chan_title not in test_data["Channels"]:
            print( "title on in test. ignore." )
            continue

        try:
            subplot = plt.subplot( n_figures_x, n_figures_y, idx ) 
            ref_x_len = len( ref_data["Channels"][chan_title]["Data"] )
            tst_x_len = len( test_data["Channels"][chan_title]["Data"] )
            ref_tstep = 1
            tst_tstep = 1
            if "Simulation_Timestep" in ref_data["Header"].keys():
                ref_tstep = ref_data["Header"]["Simulation_Timestep"]
                if "Simulation_Timestep" in test_data["Header"].keys():
                    tst_tstep = test_data["Header"]["Simulation_Timestep"]
            ref_x_data = np.arange( 0, ref_x_len*ref_tstep, ref_tstep )
            tst_x_data = np.arange( 0, tst_x_len*tst_tstep, tst_tstep )
            subplot.plot( ref_x_data, ref_data["Channels"][chan_title]["Data"], 'r-', tst_x_data, test_data["Channels"][chan_title]["Data"], 'b-' )
            plt.setp( subplot.get_xticklabels(), fontsize='5' )
            plt.title( chan_title, fontsize='6' )
            idx += 1
        except Exception as ex:
            print( "Exception: " + str(ex) )

    if reference==comparison:
        plt.suptitle( label + " " + reference)
    else:
        plt.suptitle( label + " reference(red)=" + reference + "  \n test(blue)=" + comparison )
    plt.subplots_adjust( bottom=0.05 )
    mng = plt.get_current_fig_manager()
    #mng.full_screen_toggle()
    #if os.name == "nt":
    #    mng.window.state('zoomed')
    #else:
        #mng.frame.Maximize(True)
    if "savefig" in sys.argv:
        path_dir = label.split( os.path.sep )[0]
        plotname = label.split( os.path.sep )[1]
        pylab.savefig( os.path.join( path_dir, plotname ) + ".png", bbox_inches='tight', orientation='landscape' ) #, dpi=200 )
    plt.show()

def main( reference, comparison, label ): 
    plotCompareFromDisk( reference, comparison, label )
    #plotBunch()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('reference', help='Reference chart(s) filename')
    parser.add_argument('comparison', default=None, nargs='?', help='Comparison chart(s) filename')
    parser.add_argument('label', default='', nargs='?', help='Plot label')
    args = parser.parse_args()

    main(args.reference, args.comparison if args.comparison else args.reference, args.label)
