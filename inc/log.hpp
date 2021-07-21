#ifndef LOG_H
#define LOG_H

#include "logslice.hpp"
#include "simplelog.hpp"
#include "regularlog.hpp"
#include "periodiclog.hpp"
#include "circularlog.hpp"
#include "logutility.hpp"

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
// TODO: namespace
// TODO: arvestada driftiga? - räägi AOCS inimestega

// More important stuff:
// TODO: CircularLog
// TODO: pokumentatsioon

// NOTE: metafile ülekirjutamine on ok
// NOTE: use references, whenever possible
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
// 8. Implementeerida päris timestamp koodis

template<template <class> class T, class E> requires Loggable<T,E>
class Log {
    private:
        void* obj;
    public:
        /**** Constructors ****/

        Log(T<E>* log_obj) {
            this->obj = log_obj;
        };

        /**** Getters ****/

        void* get_obj() {
            return obj;
        }

        log_decode_info_t get_decode_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_decode_info();
        }

        uint8_t* get_file() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file();
        }

        uint8_t* get_indexfile() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_indexfile();
        }

        uint8_t* get_metafile() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_metafile();
        }

        // Get size of log file in bytes
        uint32_t get_file_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file_size();
        }

        // Get size of log indexfile in bytes
        uint32_t get_indexfile_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_indexfile_size();
        }

        /**** Main functionality ****/

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

        // Read an array of log entries from the chosen time period
        // Write resulting log slice into the file new_file
        LogSlice<T,E> slice(time_t start_ts, time_t end_ts, uint8_t* new_file) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->slice(start_ts, end_ts, new_file);
        }

        /**** Utility functions ****/

        // Write all datapoints in volatile memory to file (valid for everything except SimpleLog)
        void flush() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->flush();
        }

        // Get resolution of log timestamps (valid only for RegularLog)
        int8_t get_resolution() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_resolution();
        }

        // Signify period change in incoming data (valid only for PeriodicLog)
        void period_change() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->period_change();
        }

        // Write current state to metafile (valid for all)
        void save_meta_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->save_meta_info();
        }
};

#endif
