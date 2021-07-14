#ifndef LOG_H
#define LOG_H

#include "logslice.h"
#include "simplelog.h"
#include "regularlog.h"
#include "periodiclog.h"
#include "circularlog.h"
#include "logutility.h"

/**
 * @file logging.h
 *
 * Module for data logging on ESTCube-2
 * Logging module configuration can be found in @file logging_cfg.h
 *
 * The module allows to:
 * - Create a log in a given file
 * - Save datapoints and their capturing times in the created log file
 * - Search for data logged in a given time interval from the log file
 */

// Smaller stuff:
// TODO: use references
// TODO: kahendotsing templatest välja
// TODO: slice f-nile arg, nö "kuhu mind kirjutatakse" (hiljem)

// More important stuff:
// TODO: getter resolution jaoks
// TODO: resolution slicemisel arvesse võtta
// TODO: logifaili algusesse logi tüüp (simple, regular, jne)
// TODO: iga klass eraldi h-faili
// TODO: decoder uuendada
// TODO: PeriodicLog

// NOTE: metafile ülekirjutamine on ok
// NOTE: for future ref: ära salvesta käsitsi failisuurust

// Task list
// 1. Logi decoder refactor + test siinusandmetega
// 1.5. Log object tagastab logi metadata, kus on lisaks decode infole internal state (kui toimub powerout). Logi konstruktor võtab pointeri metafailile.
// 2. Ettevalmistused failisüsteemi tulekuks
// 3. Logi sisukord (iga 10 TS tagant kirje). Kujul timestamp-tema aadress failis(mitmes bait). Optional (kui metafaili ei anta, ei tehta).
// Hoitakse nii Log objekti väljana kui metafailina (peab säilima pärast powerouti).
// 4. CRC iga logi entry kohta. Optional. Otsi CRCde kohta - mis on meile parim? Mathias pakub CRC8. Triple buffer???
// 5. Slicing. Virtuaalsed sliced. Pointer algustimestampile ja pointer lõputimestampile (lähimale järgnevale) failis. Dokumentatsioon!
// Slice peab inheritima koos Log objektiga sama interface'i, kus on compress f-n ja vb midagi veel. Slice-l peab olema võimalus päris logiks saada (kopeerimine).
// 6. SliceLog objektil on compress funktsioon. Loetakse antud aegadega piiratud andmed ja siis compressitakse. Kui kasutaja annab faili, kirjutatakse tulemus faili.
// 7. Kolm logi tüüpi: täiesti random (timestamp + data), somewhat random aga enamasti perioodiline (ts + n * (data + timedelta)), perioodiline (PeriodicLog)

template<template <class> class T, class E> requires Loggable<T,E>
class Log {
    private:
        void* obj;
    public:
        Log(T<E>* log_obj) {
            this->obj = log_obj;
        };

        // Get size of log file in bytes
        uint32_t get_file_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file_size();
        }

        // Write current state to metafile
        void save_meta_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->save_meta_info();
        }

        // Write all datapoints in volatile memory to file
        void flush() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->flush();
        }

        // Write given datapoint with given timestamp to log file
        void log(E& data, time_t timestamp) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->log(data, timestamp);
        };

        // Read an array of log entries from the chosen time period
        LogSlice<T,E> slice(time_t start_ts, time_t end_ts) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->slice(start_ts, end_ts);
        }
};

#endif
