import firebase_admin
from firebase_admin import db
import json
import os
import threading
from data_gen import *

os.chdir("C:\Codes\ESP32\Fuel_Monitor\py")
database_url = "https://fuel-consumption-742fd-default-rtdb.europe-west1.firebasedatabase.app/"
cred_obj = firebase_admin.credentials.Certificate("fuel-consumption-742fd-firebase-adminsdk-3m1ly-76946a36a7.json")
default_app = firebase_admin.initialize_app(cred_obj, {'databaseURL':database_url})



class fbClient():
    def __init__(self,path) -> None:
        self.path = path
        ref = db.reference(self.path)
        self.data = ref.get()
        
    def getDb(self):
        ref = db.reference(self.path)
        self.data = ref.get()
    def updateDb(self,data):
        ref = db.reference(self.path)
        ref.set(data)
    def listen_callback(self,event):
        print(event.path.split("/"))
    def listen(self):
        self.listening = True
        self.listener = db.reference("/").listen(self.listen_callback)
        
    
    def listenerClose(self):
        if self.listening:
            self.listener.close()
            self.listening = False
            print("Listening closed\n")

import qrcode

# cars = randomBrands(25)
# ids = randomVehicleIds(25)
# dict = {"Base Camp 0":{}}
# for i in range(len(ids)):
#     dict["Base Camp 0"][ids[i]] = [cars[i],"2","25L","100L"]
# dict["Base Camp 1"] = {}
# cars = randomBrands(18)
# ids = randomVehicleIds(18)
# for i in range(len(ids)):
#     dict["Base Camp 1"][ids[i]]= [cars[i],"2","25L","100L"]

# fb = fbClient("/")
# fb.updateDb(dict)
