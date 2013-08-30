
BEGIN {
    id = 0;
    hwver = 0;
    fwver = 0;
    swver = 0;
    usbid = "";
    mac = 0;
    host = "";
    comment = "";

    print "id  HWver   FWver SWver USB id     MAC address       hostname     comments";
    print "--- ------- ----- ----- ---------- ----------------- ------------ ---------------------------------------";
}

/Board id/ { id = $3 }

/HW version/ { hwver = $3 }

/FW version/ { fwver = $3 }

/SW version/ { swver = $3 }

/USB id/ { usbid = $3 }

/MAC address/ {
    macraw = $3;
    mac = substr(macraw,1,2) ":" substr(macraw,3,2) ":" substr(macraw,5,2) ":" substr(macraw,7,2) ":" substr(macraw,9,2) ":" substr(macraw,11,2);
}

/Hostname/ { host = $2 }

/Comment/ { comment = $2 }

/CLOSE/ {
    printf "%3d %7s %5s %5s %10s %17s %12s %s\n", id, hwver, fwver, swver, usbid, mac, host, comment;
}


