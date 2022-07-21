import smtplib
from email.message import EmailMessage
import imghdr
import os
os.chdir("C:\Codes\ESP32\Fuel_Monitor\py")
address = "wameedh.reg@gmail.com"
password = "WameedhC001A"
def sendQrImages(qr_list,address,password,target_adress):
    msg = EmailMessage()
    msg["Subject"] = "QR"
    msg["From"] = address
    msg["To"] = "dahmadjid@gmail.com" 
    msg.set_content("QR Codes for base camp 0")
    for img in qr_list:
        with open(img,"rb") as f:
            file_data = f.read()
            file_type = imghdr.what(f.name) 
            file_name = f.name

        msg.add_attachment(file_data,maintype="image",subtype = file_type,filename = file_type)

    with smtplib.SMTP_SSL("smtp.gmail.com",465) as s:

        s.login(address,password)
        s.send_message(msg)
