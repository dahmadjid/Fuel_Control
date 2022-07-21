from logging import disable
from re import X
import kivy
from kivy.config import Config
from kivy.uix.floatlayout import FloatLayout
from kivymd.uix.behaviors import elevation
from kivymd.uix.dialog.dialog import MDDialog
Config.set('input', 'mouse', 'mouse,multitouch_on_demand')
Config.set('graphics', 'resizable', False)
import os, sys,json
from kivymd.app import MDApp
from kivymd.uix.screen import MDScreen
from kivy.clock import Clock
from kivy.uix.textinput import TextInput
from kivy.uix.popup import Popup
from kivymd.uix.gridlayout import MDGridLayout
from kivy.metrics import dp,MetricsBase
from kivymd.uix.datatables import MDDataTable
metricsbase = MetricsBase()
from kivymd.uix.floatlayout import MDFloatLayout
from kivy.utils import get_color_from_hex as hex
from kivy.core.window import Window
from kivy.uix.screenmanager import ScreenManager,NoTransition
from kivy.metrics import dp
from kivymd.app import MDApp
from kivymd.uix.datatables import MDDataTable
from kivymd.uix.screen import MDScreen
from kivymd.uix.button import MDFillRoundFlatIconButton, MDFlatButton,MDRaisedButton, MDFloatingActionButtonSpeedDial, MDIconButton, MDRectangleFlatIconButton, MDRoundFlatIconButton,MDFloatingActionButton
from kivymd.uix.textfield import MDTextField, MDTextFieldRect, MDTextFieldRound
from kivymd.uix.label import MDLabel,MDIcon
from fb_client import *
from kivy.uix.label import Label
from kivymd.uix.selectioncontrol import MDCheckbox
import qrcode
from kivymd.uix.pickers.datepicker import MDDatePicker
from data_gen import *
from functools import partial
import datetime
import smtplib
from email.message import EmailMessage
import imghdr
# time table display according to day or smth
# make the dp dependent on the number of characters
print(os.getcwd())
home = "C:\Codes\ESP32\Fuel_Monitor\py"
os.chdir(home)
with open("generated.json") as f:
    data_dict = json.load(f)
data_dict = data_dict["employees"]
empty = {}
for item in data_dict:
    empty[item["name"]] = item["consumption"]

data_dict = empty
# data_list = []
font = "Oswald-Light.ttf" 
font_bold = "Oswald-Regular.ttf"
enabled_blue = "##025ae2"  
good_green = "#5df2d6"
def removeSigns(string): #this removes signs that doesnt work with file names
    newstring = ''
    for char in string:
        if char not in '\/<>*:?"|':
            newstring = newstring + char
    return newstring
class FloatGrid(FloatLayout):

    def __init__(self, height_wid,**kwargs):
        super().__init__(**kwargs)
        self.height_wid = height_wid
        self.wid_num = 0
    def add(self,wid):
       
        wid.pos_hint = {"x" : 0,"y" :1 - self.height_wid*(1 + self.wid_num)}
        wid.size_hint = (1,self.height_wid)
        self.add_widget(wid)
        self.wid_num += 1


class TextInputGrid(FloatLayout):
    def __init__(self,lbl_text = "",input_text = "", **kwargs):
        super().__init__(**kwargs)
        self.label = MDLabel(text = lbl_text,font_name = font ,font_size = 16,size_hint = (0.2,1),pos_hint = {"x": 0,"y":0},halign="center",theme_text_color= "Custom" ,text_color = hex("000000"))
        self.input = MDTextField(text =input_text, line_color_normal = hex("e1e1e1"),line_color_focus=hex(enabled_blue),color_mode = "custom",size_hint = (0.3,1),pos_hint = {"x": 0.25,"y":-0.25})
        self.add_widget(self.label)
        self.add_widget(self.input)

        self.input.helper_text = ""
        self.input.helper_text_mode = "on_error"
class Root(MDFloatLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.checked_rows = []
        self.loadDataFromDisk()
        self.address = "qrcodesender0@gmail.com"
        self.password = "qr13102001"
        Window.maximize()
        max = Window.size
        Window.size = (max[0]*0.8,max[1]*0.82) #0.865
        Window.top = (max[1] - Window.size[1])/2 + Window.size[1]/16
        Window.left = (max[0] - Window.size[0])/2 
        Window.borderless = True
        # root = MDFloatLayout(md_bg_color = hex(enabled_blue))
        # top_bar_flt = MDFloatLayout(md_bg_color =hex("ffffff"),size_hint=(1,0.05),pos_hint ={"x" : 0,"y" : 0.95})
        flt = MDFloatLayout(md_bg_color = hex("#f1f1f1"))#,size_hint=(1,0.95),pos_hint ={"x" : 0,"y" : 0})
        filter_flt = MDFloatLayout(md_bg_color = hex("#ffffff"),size_hint=(0.85,0.1),pos_hint ={"x" : 0.15,"y" : 0.9})
        table_flt = MDFloatLayout(md_bg_color = hex("#f7f7f7"),size_hint=(0.85,0.9),pos_hint ={"x" : 0.15,"y" : 0})
        self.tab_flt = MDFloatLayout(md_bg_color = hex(enabled_blue),size_hint=(0.15,1),pos_hint ={"x" : 0,"y" : 0})
        self.table = self.dataTable(0.85,5)
        self.data_obj = self.table.get()
        table_flt.add_widget(self.table)
        filter_flt.add_widget(self.filterTab())
        self.menu_tab = self.menuTab()
        self.tab_flt.add_widget(self.menu_tab)
        flt.add_widget(filter_flt)
        flt.add_widget(table_flt)
        flt.add_widget(self.tab_flt)
        self.thread = threading.Thread(target = self.fbthread)
        self.thread.start()
        # root.add_widget(flt)
        # root.add_widget(top_bar_flt)
        
        # return root
       
        
        self.add_widget(flt)
    def fbthread(self):
        self.fb = fbClient("/")
        self.fb.listen_callback = self.listen_callback
        self.fb_running = True
        self.data_dict = self.fb.data
        #self.table.update_row_data(None,self.centerRowDataText(self.dictDataToRowData(self.data_dict[self.base_camp])))
        self.fb.listen()
        self.saveDataOnDisk()
    def listen_callback(self,event):
        print("Connected\n")
        self.data_dict = self.fb.data
        data_list = self.dictDataToRowData(self.data_dict[self.base_camp])
        self.table.update_row_data(None,self.centerRowDataText(data_list))
        self.saveDataOnDisk()
    def loadDataFromDisk(self):
        with open("data.json") as f:
            self.data_dict=json.load(f)
            self.base_camps = []
        for key,value in self.data_dict.items():
            self.base_camps.append(key)
        self.base_camp = self.base_camps[0]
    def saveDataOnDisk(self):
        with open("data.json","w") as f:
            json.dump(self.data_dict,f)

    def on_save(self, instance, value, date_range):

        self.date_picked = value
        print(instance, value, date_range)

    def on_cancel(self, instance, value):
        '''Events called when the "CANCEL" dialog box button is clicked.'''

    def show_date_picker(self,btn):

        date_dialog = MDDatePicker(min_year = 2021 , max_year = 2035)
        date_dialog.bind(on_save=self.on_save, on_cancel=self.on_cancel)
        date_dialog.open()
    def onCheck(self,instance,row):
        
            if row in self.data_obj.checked_rows:
                print(row,self.data_obj.checked_rows[self.data_obj.checked_rows.index(row)])
                self.data_obj.checked_rows.pop(self.data_obj.checked_rows.index(row))
                
            else:
                print(row)
                self.data_obj.checked_rows.append(row)
                
            
            print(len(self.data_obj.checked_rows))
    
    def onRowPress(self,instance_table, instance_row):
        row_index = int(instance_row.index / len(self.col_data)) #- len(self.col_data) * (instance_row.index % len(self.col_data))
        page = self.table.table_data._rows_number
        index = row_index + page * self.table.rows_num
        self.vehicle_id = self.row_data[index][0]
        data = self.data_dict[self.base_camp][self.vehicle_id] 
        
        print(data)
        
        height = Window.size[1]/5   
        dp_height= dp(height*0.25)
        flt = FloatLayout(pos_hint = {"x":0,"y":0},height = dp_height*2,size_hint = (0.5,None))
        # self.email_input = MDTextFieldRound(color_active = hex("ffffff"),line_color=hex(enabled_blue),normal_color = hex("ffffff"),icon_left = "email",hint_text = "john@example.com,jane@example.com..")
        # flt.add(self.email_input)
        self.per_day_input = TextInputGrid(lbl_text="Uses/Day :",input_text=data[1],size_hint = (0.85,0.5),pos_hint = {"x":0.075,"y":0.5})
        self.per_use_input = TextInputGrid(lbl_text="Liter/Use :",input_text=data[2][:-1],size_hint = (0.85,0.5),pos_hint = {"x":0.075,"y":0})
        flt.add_widget(self.per_day_input)
        flt.add_widget(self.per_use_input)
        
        
        cancel_btn = MDRaisedButton(text="Cancel", text_color=hex(enabled_blue),md_bg_color = hex("ffffff"))
        ok_btn = MDRaisedButton(text="Update & Send", text_color=hex("ffffff"),md_bg_color = hex(enabled_blue))
        ok_btn.bind(on_release = self.onOkEdit)
        self.editRowDialog = MDDialog(title = "Edit Quota:",type = "custom",content_cls = flt,buttons =[cancel_btn,ok_btn])
        cancel_btn.bind(on_release = partial(self.onCancel,dialog = self.editRowDialog))
        self.editRowDialog.update_height()
        self.editRowDialog.update_width()
        self.editRowDialog.open()
    def onOkEdit(self,btn):
        per_day = self.per_day_input.input.text
        per_use = self.per_use_input.input.text
        
        try:
            int(per_day)
        except:
            self.per_day_input.input.error = True
            return
        try:
            int(per_use)
        except:
            self.per_use_input.input.error = True
            return
        data = self.data_dict[self.base_camp][self.vehicle_id]
        self.data_dict[self.base_camp][self.vehicle_id] = [data[0],per_day,per_use+"L",data[3]]
        self.row_data = self.dictDataToRowData(self.data_dict[self.base_camp])
        self.table.update_row_data(None,self.centerRowDataText(self.row_data))
        self.saveDataOnDisk()
        self.fb.updateDb(self.data_dict)
        self.editRowDialog.dismiss()
        
    def dataTable(self,width_hint,col_num):
        width = Window.size[0]/5
        dp_width = dp(width*width_hint/col_num)
        
        
        self.row_data = self.dictDataToRowData(self.data_dict["Base Camp 0"])
        self.col_data =[("Vehicle ID",dp_width),("     Vehicle Brand", dp_width),("Uses/Day", dp_width),("Liter/Use", dp_width),("Fuel Consumption", dp_width)]

            
        # def sort(data,col = 0):
        #     return zip(
        #         *sorted(
        #             enumerate(data),
        #             key=lambda l: l[1][col]
        #         )
        #     ) 
        color =hex(enabled_blue)
        color[3] = 0.1
        data_table = MDDataTable(
            use_pagination=True,
            check=True,
            column_data=self.col_data,
            row_data = self.centerRowDataText(self.row_data),
            sorted_on="Employee Name",
            sorted_order="ASC",
            elevation=2,
            pos_hint = {"x" : 0,"y" : 0},
            rows_num = 10,
            # background_color_header=color,
            # background_color_cell=hex("#451938"),
            
            background_color_selected_cell=color
        )
        data_table.bind(on_check_press = self.onCheck)
        data_table.bind(on_row_press=self.onRowPress)
        return data_table

    def filterTab(self):
        flt = MDFloatLayout(pos_hint = {"x" : 0,"y": 0},md_bg_color = hex("fafafa"))
        search_bar = MDTextFieldRound(color_active = hex("ffffff"),line_color=hex(enabled_blue),normal_color = hex("ffffff"),icon_left = "database-search",hint_text = "Search",size_hint = (0.23,0.5),pos_hint = {"x" : 0.39,"y":0.25})
        gen_qr_btn = MDFillRoundFlatIconButton(text = "Update QR",font_size = 18,font_name = font_bold,md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),icon = "qrcode-plus",size_hint = (0.2,0.5),pos_hint = {"x" : 0.74,"y":0.25})
        remove_qr_btn = MDFillRoundFlatIconButton(text = "Remove QR",font_size = 18,font_name = font_bold,icon = "qrcode-remove",md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),size_hint = (0.2,0.5),pos_hint = {"x" : 0.87,"y":0.25})
        #lbl = MDLabel(text = "Fuel Consumption",pos_hint = {"x":0.1,"y":0.25},size_hint = (0.4,0.5),font_style = "H5",text_color = hex("ffffff"))
        new_btn = MDFillRoundFlatIconButton(text = "New",font_size = 18,font_name = font_bold,icon = "plus",md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),size_hint = (0.2,0.5),pos_hint = {"x" : 0.65,"y":0.25})
        graph_btn = MDFillRoundFlatIconButton(text = "Graph",font_size = 18,font_name = font_bold,md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),icon = "chart-bell-curve-cumulative",size_hint = (0.2,0.5),pos_hint = {"x" : 0.025,"y":0.25})
        settings_btn = MDFillRoundFlatIconButton(text = "Settings",font_size = 18,font_name = font_bold,icon = "database-settings",md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),size_hint = (0.2,0.5),pos_hint = {"x" : 0.125,"y":0.25})
        date_btn = MDFillRoundFlatIconButton(text = "Date/Time",font_size = 18,font_name = font_bold,icon = "clock-time-three-outline",md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),size_hint = (0.2,0.5),pos_hint = {"x" : 0.24,"y":0.25})
        date_btn.bind(on_release = self.show_date_picker)
        gen_qr_btn.bind(on_release = self.onUpdateQr)
        flt.add_widget(date_btn)
        flt.add_widget(new_btn)
        flt.add_widget(remove_qr_btn)
        flt.add_widget(gen_qr_btn)
        flt.add_widget(search_bar)
        flt.add_widget(graph_btn)
        flt.add_widget(settings_btn)
        #flt.add_widget(lbl)
        return flt    
    def menuTab(self):

        flt = MDFloatLayout(pos_hint = {"x" : 0,"y": 0},md_bg_color = hex(enabled_blue),size_hint = (1,1))
        #bases_label = MDLabel(text = "Base Camps",font_name = font ,font_size = 20,size_hint = (1,0.05),pos_hint = {"x": 0,"y":0.93},halign="center",theme_text_color= "Custom" ,text_color = hex("ffffff"))
        #bases_icon = MDIcon(icon = "home-group",size_hint = (0.2,0.2),pos_hint = {"x":0.05,"y":0.855},text_color = hex("ffffff"),theme_text_color= "Custom")
        #bases_plus_btn = MDIconButton(icon = "plus",pos_hint = {"x":0.72,"y":0.919},md_bg_color = hex(enabled_blue),text_color = hex("ffffff"),theme_text_color= "Custom")
        grid = FloatGrid(pos_hint = {"x" : 0,"y":0},size_hint = (1,0.9),height_wid = 0.1)
        self.camps_btn = []
        for camp in self.base_camps:
            btn = MDFillRoundFlatIconButton(icon = "home",text = camp ,theme_text_color= "Custom" ,text_color = hex("ffffff"),font_size = 18,font_name = font,md_bg_color = hex(enabled_blue))
            btn.bind(on_release = self.onCamp)
            self.camps_btn.append(btn)
            grid.add(btn)
        self.camps_btn[0].md_bg_color = hex("#025ad2")

        new_btn = MDFillRoundFlatIconButton(text = "Add Base Camp",font_size = 18,font_name = font,icon = "plus",md_bg_color = hex(enabled_blue),icon_color = hex("ffffff"),text_color = hex("ffffff"),size_hint = (0.2,0.5),pos_hint = {"x" : 0.65,"y":0.25})
        new_btn.bind(on_release = self.addCamp)
        grid.add(new_btn)

        # flt.add_widget(bases_plus_btn)
        # flt.add_widget(bases_label)
        # flt.add_widget(bases_icon)
        flt.add_widget(grid)
        
        return flt
    def addCamp(self,btn):

        height = Window.size[1]/5
        dp_height= dp(height*0.25)
        flt = FloatGrid(height_wid = 1,pos_hint = {"x":0.15,"y":0},height = dp_height,size_hint = (0.7,None))
        self.camp_input = MDTextFieldRound(color_active = hex("ffffff"),line_color=hex(enabled_blue),normal_color = hex("ffffff"),icon_left = "home",hint_text = "Base Camp Name")
        flt.add(self.camp_input)
        cancel_btn = MDRaisedButton(text="Cancel", text_color=hex(enabled_blue),md_bg_color = hex("ffffff"))
        
        ok_btn = MDRaisedButton(text="OK", text_color=hex("ffffff"),md_bg_color = hex(enabled_blue))
        ok_btn.bind(on_release = self.onOkCamp)
        self.camp_dialog = MDDialog(title = "Add Base Camp",type = "custom",content_cls = flt,buttons =[cancel_btn,ok_btn])
        cancel_btn.bind(on_release = partial(self.onCancel,dialog = self.camp_dialog))
        self.camp_dialog.update_height()
        self.camp_dialog.update_width()
        self.camp_dialog.open()
    def onUpdateQr(self,btn):
        check_flt = FloatLayout()
        height = Window.size[1]/5
        dp_height= dp(height*0.25)
        flt = FloatGrid(height_wid = 0.5,pos_hint = {"x":0.15,"y":0},height = dp_height*2,size_hint = (0.7,None))
        self.email_input = MDTextFieldRound(color_active = hex("ffffff"),line_color=hex(enabled_blue),normal_color = hex("ffffff"),icon_left = "email",hint_text = "john@example.com,jane@example.com..")
        flt.add(self.email_input)
        check = MDCheckbox(size_hint = (0.05,0.5),pos_hint = {"x": 0,"y":0.25})
        flt.add(check_flt)
        remove_previous_label = MDLabel(text = "Disable previous QR codes",font_name = font ,font_size = 16,size_hint = (0.5,0.5),pos_hint = {"x": 0.025,"y":0.25},halign="center",theme_text_color= "Custom" ,text_color = hex("000000"))
        check_flt.add_widget(check)
        check_flt.add_widget(remove_previous_label)

        cancel_btn = MDRaisedButton(text="Cancel", text_color=hex(enabled_blue),md_bg_color = hex("ffffff"))
        
        ok_btn = MDRaisedButton(text="Update & Send", text_color=hex("ffffff"),md_bg_color = hex(enabled_blue))
        ok_btn.bind(on_release = self.onOkUpdateQR)
        self.update_qr_dialog = MDDialog(title = "Update QR codes of the selected and send email to :",type = "custom",content_cls = flt,buttons =[cancel_btn,ok_btn])
        cancel_btn.bind(on_release = partial(self.onCancel,dialog = self.update_qr_dialog))
        self.update_qr_dialog.update_height()
        self.update_qr_dialog.update_width()
        self.update_qr_dialog.open()
    def onOkUpdateQR(self,btn):

        check_list = self.data_obj.checked_rows
        self.target = self.email_input.text
        # print(check_list,len(check_list))
        self.qr_list = []
        for row in check_list:
            file_name = "qr_"+row[0].strip().replace(" ","") +"_"+row[1].strip().replace(" ","_")+"_"+str(datetime.datetime.today()).replace(":","_").replace(" ","_")+".jpg"
            file_name = removeSigns(file_name)
            img = qrcode.make(file_name)
            img.save(file_name)
            self.qr_list.append(file_name)
        thread = threading.Thread(target=self.sendQrImages)
        thread.start()
        self.update_qr_dialog.dismiss()
        
    def onOkCamp(self,btn): 
        self.table.header.ids.check.state = "normal"
        self.base_camps.append(self.camp_input.text)
        self.tab_flt.remove_widget(self.menu_tab)
        self.menu_tab = self.menuTab()
        self.tab_flt.add_widget(self.menu_tab)
        self.data_dict[self.camp_input.text] = {}
        self.camp_dialog.dismiss()

    def onCancel(self,btn,dialog=None):
        dialog.dismiss()
    def onCamp(self,btn):
        self.data_obj.check.state = "normal"   
        self.table.table_data.select_all("normal")
        self.row_data = self.dictDataToRowData(self.data_dict[btn.text])
        self.table.update_row_data(None,self.centerRowDataText(self.row_data))
        self.base_camp = btn.text
        for camp in self.camps_btn:
            if btn != camp:
                camp.md_bg_color = hex(enabled_blue)
            else:
                camp.md_bg_color = hex("#025ad2")
        print(self.base_camp)
        # print(self.data_obj.checked_rows)
    def centerRowDataText(self,data_list):
        i = 0
        for text,dp_width in self.col_data:
            for j in range(len(data_list)):

                space_num =len(str(text))-len(str(data_list[j][i]))
                data_list[j][i] = " "*space_num + str(data_list[j][i])

            i+=1
        return data_list
    
    def dictDataToRowData(self,dict):
        row_data = []
        for key,value in dict.items():
            lis = [key]
            for elem in value:
                lis.append(elem)
            row_data.append(lis)
        return row_data

    def sendQrImages(self):
        msg = EmailMessage()
        msg["Subject"] = "QR"
        msg["From"] = self.address
        msg["To"] = "dahmadjid@gmail.com"
        msg.set_content("QR Codes for: "+self.base_camp)
        print("Adding Attachements")
        for img in self.qr_list:
            with open(img,"rb") as f:
                file_data = f.read()
                file_type = imghdr.what(f.name) 
                file_name = f.name

            msg.add_attachment(file_data,maintype="image",subtype = file_type,filename = file_name)
        
        with smtplib.SMTP_SSL("smtp.gmail.com",465) as s:

            s.login(self.address,self.password)
            s.send_message(msg)
        print("Email Sent Successfully")

class FuelMonitor(MDApp):
    def build(self):
        self.root = Root()

        return self.root
app=FuelMonitor()
app.run()
app.root.fb.listenerClose()
app.root.thread.join()