<h1 align="center">Introduction to IOT Smart Locker System Project</h1>

## Overview ![Overview](https://img.shields.io/badge/Project-Overview-blue?style=flat-square&logo=info&logoColor=white)
"นี้เป็นชิ้นงานโปรเจคในรายวิชา Introduction to IOT ของภาควิชา วิศวกรรมระบบไอโอทีและสารสนเทศ ลาดกระบัง โดยหัวข้อโปรเจคคือนำบอร์ด Arduino UNO R4 WiFi ไปออกแบบเป็นชิ้นงานออกมา 1 โปรเจคซึ่งกลุ่มผมเลือกทำระบบ Locker อัจฉริยะเนื่องจากกลุ่มผมเห็น Pain Point จ่ากตอนปี 1 เวลาที่มีกิจกรรมที่คณะหรือไปทำธุระต่างๆ เราต้องเอาสัมภาระไปวางไว้ตามพื้นหรือที่ต้างๆ ทำให้ของมีค่าหรือกระเป๋าไม่มีความปลอดภัยจากบุคคลภายนอกทำให้อาจมีมิจฉาชีพมาขโมยกระเป๋าหรือของมีค่าได้ เราจึงนำ Pain Point หรือปัญหาตรงนี้มาเป็นหัวข้อและออกแบบโปรเจคที่ให้ Solution ของปัญหานี้ โดยตัว Locker อัจฉริยะนี้จะเป็น Prototype System ที่เราจะออกแบบระบบโดยคำนึงถึงการใช้จริงโดยตอนแรกกลไกการทำงานของ Locker จะเป็นคล้ายๆ แผ่น Load cell ที่จะมีเส้นวงจรอยู่ด้านล่างกล่องแล้วเมื่อวางของลงไปกล่องจะส่ง Signal ไปยัง Controller ทำให้เรารับรู้ได้ว่ามีสิ่งของอยู่ในกล่องโดยเมื่อในกล่องถูกใส่สิ่งของเข้าไปครั้งแรกจะขึ้นให้เราใส่ Onetime Password ที่จะใช้กับกล่องนี้โดยตลอดระยะเวลาที่กล่องนี้มีของอยู่มันก็จะจำรหัสผ่านนี้ไว้โดยเมื่อใดก็ตามที่ผู้ใช้เลิกใช้กล่องนี้แล้วหรือนำของออกจากกล่อง Locker นี้มันก็จะนับเวลาถอยหลัง 3 นาทีและรีเซ็ตรหัสผ่านและปลดล็อคกล่องนี้เปิดโอกาสให้ User คนต่อไปได้ใช้กล่องนี้ต่อโดยตัวกล่องจะมีระบบ Monitor แบบ Realtime ขึ้น Dashboard ไว้คอยให้ผู้มีส่วนดูแลหรือ User คนอื่นสามารถดูได้ว่ากล่องไหนถูกใช้ไปแล้วหรือกล่องไหนยังว่างอยู่บ้าง"

## Tech Stack ![TechStack](https://img.shields.io/badge/System-Architecture_&_Tools-8E44AD?style=flat-square&logo=cpu&logoColor=green)
- $\color{cyan}{\textbf{Hardware :}}$ Arduino UNO R4 WIFI
- $\color{orange}{\textbf{Firmware :}}$ C++ (Arduino Framework), Node-red

## Concept ![Concept](https://img.shields.io/badge/Concept-red?style=flat-square)
เราจะแบ่งระบบการทำงานอยู่ 2 ส่วน
- $\color{red}{\textbf{Pannel Controller}}$ ในส่วนของบอร์ด Pannel จะมีระบบ Monitor ว่า Locker ไหนถูกใช้ไปแล้วบ้างและจะมีระบบ กรอกรหัสผ่านหรือจำรหัสผ่านที่เป็น Onetime Password และสื่อสารกับ Cloud ว่า Locker ไหนยังว่างอยู่
- $\color{green}{\textbf{Locker Controller}}$ ในส่วนของ Locker Contoller จะมีระบบตรวจจับสิ่งของที่อยู่ใน Locker ว่า Locker มีสิ่งของอยู่รึป่าวและเมื่อไม่มีใครใช้แล้วก็จะสั่ง Reset รหัสผ่านและส่งค่าสถาณะขึ้น Cloud ว่า Locker ใหนใช้ได้บ้าง

## Contributors ![Contributors](https://img.shields.io/badge/Group-Contributors-2ECC71?style=flat-square&logo=github&logoColor=white)
- $\color{red}{\textbf{Panya Triprom}}$ ([@KENASTES](https://github.com/KENASTES))
- $\color{red}{\textbf{Natthawin Taeprasert}}$ ([@natthawin0614](https://github.com/natthawin0614)

---
*Created for Educational Purpose @KMITL*