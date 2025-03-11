# **************************************************************** #
# libaprsroute - APRS header only routing library                  #
# Version 0.1.0                                                    #
# https://github.com/iontodirel/libaprsroute                       #
# Copyright (c) 2024 Ion Todirel                                   #
# **************************************************************** #

import json
import shutil

def assign_numeric_ids(routes_data):
    """Assign incremental numeric IDs to each route, starting from 1."""
    current_suffix = 1  # Start ID numbering from 1

    for route in routes_data['routes']:
        if 'id' in route:  # If ID already exists, increment it
            route['id'] = "{}".format(current_suffix)
            current_suffix += 1

def main():
    # Load the JSON data from the file
    with open('routes.json', 'r') as file:
        routes_data = json.load(file)
    
    # Save the old file as routes.old.json
    shutil.copyfile('routes.json', 'routes.old.json')
    
    # Assign incremental numeric IDs to each route
    assign_numeric_ids(routes_data)
    
    # Save the updated JSON data back to the file
    with open('routes.json', 'w') as file:
        json.dump(routes_data, file, indent=4)
    
    print("Incremental numeric IDs have been assigned and saved to 'routes.json'.")

if __name__ == "__main__":
    main()
