
#define NO_PRINTER_NAME          "No Printer"
#define POSTSCRIPT_PRINTER_NAME  "PostScript Printer"
#define PRINTER_NAME_1           "HP DeskJet 500C"
#define DEVICE_NAME_1            "cdeskjet"
#define PRINTER_NAME_2           "HP DeskJet 500C, 510, 520, and 540C (black and white)"
#define DEVICE_NAME_2            "cdjmono"
#define PRINTER_NAME_3           "Epson-compatible"
#define DEVICE_NAME_3            "epson"
#define PRINTER_NAME_4           "HP LaserJet"
#define DEVICE_NAME_4            "laserjet"
#define PRINTER_NAME_5           "HP LaserJet Plus"
#define DEVICE_NAME_5            "ljetplus"
#define PRINT_BAT_FILE_NAME      "print.bat"

     prototype choose_printer();
     prototype update_print_dot_bat( STRING, STRING );
     prototype include_ghostscript_if_needed( STRING );

     STRING      global_printer_name;

