# **************************************************************** #
# libaprsroute - APRS header only routing library                  # 
# Version 0.1.0                                                    #
# https://github.com/iontodirel/libaprsroute                       #
# Copyright (c) 2024 Ion Todirel                                   #
# **************************************************************** #

import json
import random
import string

def generate_unique_id(existing_ids):
    """Generate a unique 5-character alphanumeric ID."""
    while True:
        new_id = ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))
        if new_id not in existing_ids:
            return new_id

def assign_unique_ids(routes_data):
    """Assign unique IDs to each test case."""
    existing_ids = set()
    for route in routes_data['routes']:
        new_id = generate_unique_id(existing_ids)
        route['id'] = new_id
        existing_ids.add(new_id)

def main():
    # Load the JSON data from the file
    with open('routes.json', 'r') as file:
        routes_data = json.load(file)
    
    # Assign unique IDs to each route
    assign_unique_ids(routes_data)
    
    # Save the updated JSON data back to the file
    with open('routes_updated.json', 'w') as file:
        json.dump(routes_data, file, indent=4)
    
    print("Unique IDs have been assigned and saved to 'routes_updated.json'.")

if __name__ == "__main__":
    main()
