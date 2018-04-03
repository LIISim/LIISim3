#include "iomatlab.h"

#include <QDir>
#include <cstring>

#include "../core.h"
#include "../signal/processing/temperatureprocessingchain.h"
#include "../signal/processing/plugins/temperaturecalculator.h"

/**
 * @brief IOmatlab::IOmatlab Constructor.
 * @param parent
 * @details
 */
IOmatlab::IOmatlab(QObject *parent) :  IOBase(parent)
{
    // initialize private vars
    msgprfx = "Matlab Export: ";
    p_mode = PM_PERMRUN;
}


/**
 * @brief IOmatlab::exportSignals
 * @param rq SignalIORequest file (received from SignalManager, initiated by user in ExportDialog)
 */
void IOmatlab::exportImplementation(const SignalIORequest &rq)
{
    m_timer.start();

    if(rq.itype != MAT)
    {
        logMessage(msgprfx + "IO-request is not of type MAT! Export canceled.",INFO);
    }

    qDebug() << "Matlab Export ...";

    // create filename for IO-request
    QString fname = rq.datadir + rq.userData.value(0).toString();
    if(!fname.endsWith(".mat"))
        fname.append(".mat");


    // get selected mruns from SignalIORequest userdata
    QList<MRun*> mrun_list;
    QList<QVariant> runids = rq.userData.value(8).toList();
    for(int i = 0; i < runids.size(); i++)
    {
        MRun* mrun = Core::instance()->dataModel()->mrun(runids[i].toInt());
        if(mrun)
            mrun_list << mrun;
    }

    // TODO: parallelize loop
    for(int i = 0; i < mrun_list.size(); i++)
    {
        // check if operation has been aborted
        if(IOBase::abort_flag)
        {
            emit exportImplementationFinished(m_timer.elapsed()/1000.0);
            return;
        }

        bool success = saveRun(rq.datadir,mrun_list[i]);

        emit progressUpdate( (float)i/(float) mrun_list.size());
    }

    double time = m_timer.elapsed()/1000.0;

    logMessage(msgprfx + " done ("+QString::number(time)+ " s).");
    emit exportImplementationFinished(time);
}


void IOmatlab::checkFiles()
{
}


bool IOmatlab::saveRun(const QString &dirpath, MRun *run)
{

    QString fname = dirpath + run->getName() + ".mat";

    // qDebug() << "   TODO: save run " << fname;

    QFileInfo finfo(fname);
    QDir dir(finfo.absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // create MATLAB file
    QByteArray buf = fname.toLatin1();
    const char* cfname = buf.constData();
    mat_t *mat;
    mat = Mat_CreateVer(cfname, NULL, MAT_FT_MAT5);

    if(mat)
    {
        matvar_t * mrunStruct = getMRunVar(run);
        if(mrunStruct == 0)
        {
            logMessage(msgprfx + "failed to create MATLAB structured array of MRun!");
            qDebug() << msgprfx + "failed to create MATLAB structured array of MRun!";

            Mat_Close(mat);
            return false;
        }
        if(e_flag_matlab_compression)
            Mat_VarWrite(mat, mrunStruct, MAT_COMPRESSION_ZLIB);  //compression on
        else
            Mat_VarWrite(mat, mrunStruct, MAT_COMPRESSION_NONE);  //compression off
        Mat_VarFree(mrunStruct);
        Mat_Close(mat);
    }
    else
    {
        logMessage(msgprfx + "failed to write: "+fname);
        qDebug() << msgprfx + "failed to write: "+fname;

        return false;
    }
    return true;
}


/**
 * @brief IOmatlab::setupImport TODO
 */
void IOmatlab::setupImport()
{
    // not yet supported
}


/**
 * @brief IOmatlab::importStep TODO
 * @param mrun
 * @param fileInfos
 */
void IOmatlab::importStep(MRun *mrun, SignalFileInfoList fileInfos)
{
    // not yet supported
}


// -----------------------
// PRIVATE EXPORT HELPERS
// -----------------------

/**
 * @brief IOmatlab::mrunToMatlabStruct creates a MATLAB structured array of a measurement run object.
 * @return matvar_t pointer in matio library format
 * @details The output variable represents a matlab structured array containing all fields and signal
 * data of a MRun object. This function is called by IOmatlab::exportSignals(...).
 */
matvar_t* IOmatlab::getMRunVar(MRun *run)
{

    // output structured array
    matvar_t* structArray = 0;
    size_t struct_dims[2] = {1,1};

    // temporary element
    matvar_t *element;
    size_t edims[2] = {1,1};

    // number of fields in the mrun array
    const int nfields = 8;

    // names of fields
    const char* fieldnames[nfields] = {"name",
                                       "description",
                                       "channel_count",
                                       "temp_trace_count",
                                       "raw",
                                       "absolute",
                                       "temperature",
                                       "settings"};

    // --------------------------
    // init MRUN structured array
    // --------------------------

    structArray = Mat_VarCreateStruct("measurement_run", 2,
                                      struct_dims, fieldnames, nfields);

    if(structArray == NULL)
    {
        qDebug() << msgprfx << "failed to create structure array!";
        return 0;
    }

    // ----------
    // write name
    // ----------

    QByteArray buf = run->getName().toUtf8();
    const char* name = buf.constData();
    edims[0] = 1;
    edims[1] = std::strlen(name);
    element = Mat_VarCreate(NULL,MAT_C_CHAR,MAT_T_INT8,2,edims,(void*)name,0);

    if(element == NULL)
    {
        qDebug() << msgprfx << "failed to create element!";
        Mat_VarFree(structArray);
        return 0;
    }
    Mat_VarSetStructFieldByName(structArray,fieldnames[0],0,element);

    // -----------------
    // write description
    // -----------------

    buf = run->description().toUtf8();
    const char* descr = buf.constData();
    edims[0] = 1;
    edims[1] = std::strlen(descr);
    element = Mat_VarCreate(NULL,MAT_C_CHAR,MAT_T_INT8,2,edims,(void*)descr,0);

    if(element == NULL)
    {
        qDebug() << msgprfx << "failed to create element!";
        Mat_VarFree(structArray);
        return 0;
    }
    Mat_VarSetStructFieldByName(structArray,fieldnames[1],0,element);

    // ------------------------
    // write number of channels
    // ------------------------

    // TODO: save noChannels per signal type
    int noCh[1] = {run->getNoChannels(Signal::RAW)};
    edims[0] = 1;
    edims[1] = 1;
    element = Mat_VarCreate(NULL,MAT_C_INT32,MAT_T_INT32,2,edims,noCh,0);

    if(element == NULL)
    {
        qDebug() << msgprfx << "failed to create element!";
        Mat_VarFree(structArray);
        return 0;
    }
    Mat_VarSetStructFieldByName(structArray,fieldnames[2],0,element);

    // ------------------------
    // write number of temperature traces
    // ------------------------

    int32_t tempTraceCount[1];
    if(e_flag_tmp)
        tempTraceCount[0] = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE))->temperatureCalculatorCont();
    else
        tempTraceCount[0] = 0;

    edims[0] = 1;
    edims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, edims, tempTraceCount, 0);

    if(!element)
    {
        qDebug() << msgprfx << "failed to create element";
        Mat_VarFree(structArray);
        return 0;
    }
    Mat_VarSetStructFieldByName(structArray, fieldnames[3], 0, element);

    // ---------------------
    // write raw signal data
    // ---------------------

    if(e_flag_raw || e_flag_raw_postproc)
    {
        edims[0] = 1;
        edims[1] = 1;
        element = getSignalTypeStruct(run, Signal::RAW, e_flag_raw, e_flag_raw_postproc, e_flag_raw_stdev);

        if(!element)
        {
            MSG_ERR("IOmatlab: getSignalStruct() - raw signal");
            Mat_VarFree(structArray);
            return 0;
        }

        Mat_VarSetStructFieldByName(structArray, fieldnames[4], 0, element);
    }

    // ---------------------
    // write absolute signal data
    // ---------------------

    if(e_flag_abs || e_flag_abs_postproc)
    {
        edims[0] = 1;
        edims[1] = 1;
        element = getSignalTypeStruct(run, Signal::ABS, e_flag_abs, e_flag_abs_postproc, e_flag_abs_stdev);

        if(!element)
        {
            MSG_ERR("IOmatlab: getSignalStruct() - absolute signal");
            Mat_VarFree(structArray);
            return 0;
        }

        Mat_VarSetStructFieldByName(structArray, fieldnames[5], 0, element);
    }

    // ---------------------
    // write temperature signal data
    // ---------------------

    if(e_flag_tmp || e_flag_temp_postproc)
    {
        edims[0] = 1;
        edims[1] = 1;
        element = getSignalTypeStruct(run, Signal::TEMPERATURE, e_flag_tmp, e_flag_temp_postproc, e_flag_temp_stdev);

        if(!element)
        {
            MSG_ERR("IOmatlab: getSignalStruct() - temperature signal");
            Mat_VarFree(structArray);
            return 0;
        }

        Mat_VarSetStructFieldByName(structArray, fieldnames[6], 0, element);
    }

    // ---------------------
    // write settings
    // ---------------------

    matvar_t* settingsStruct = getSettingsStruct(run);
    if(settingsStruct)
        Mat_VarSetStructFieldByName(structArray,fieldnames[7],0,settingsStruct);

    return structArray;
}


/**
 * @brief IOmatlab::getSettingsStruct get mat-struct of run settings
 * @param run measuremt run
 * @return matlab struct variable containing relevant settings fiels
 */
matvar_t* IOmatlab::getSettingsStruct(MRun *run)
{
    // output structured array
    matvar_t* structArray = 0;
    size_t struct_dims[2] = {1,1};

    // number of fields in the settings array
#ifdef LIISIM_FULL
    const int nfields = 7;
#else
    const int nfields = 6;
#endif

    // names of fields
    const char* fieldnames[nfields] = {"liisettings",
                                       "liisettings_channel_wavelength",
                                       "nd_filter_id",
                                       "nd_filter_transmission",
                                       "laser_fluence",
                                       "pmt_gain_set",
                                   #ifdef LIISIM_FULL
                                       "pmt_gain_measured"
                                   #endif
                                      };

    structArray = Mat_VarCreateStruct("settings", 2,struct_dims,fieldnames,nfields);
    if(!structArray)
        return 0;

    // temporary element
    matvar_t *element = 0;
    size_t edims[2] = {1,1};

    // -----------------------
    // 0) LIISettings filename
    // -----------------------

    QString liisfname = run->liiSettings().filename;
    QByteArray buf = liisfname.toLatin1();
    const char* liisfname_charp = buf.constData();

    edims[0] = 1;
    edims[1] = liisfname.length();
    element = Mat_VarCreate(NULL,MAT_C_CHAR,MAT_T_INT8,2,edims,(void*)liisfname_charp,0);

    if(element != NULL)
        Mat_VarSetStructFieldByName(structArray,fieldnames[0],0,element);

    // -----------------------
    // 1) LIISettings channel wavelength
    // -----------------------

    QVector<double> wavelengths;
    for(int i = 0; i < run->liiSettings().channels.size(); i++)
        wavelengths.append(run->liiSettings().channels.at(i).wavelength);

    edims[0] = 1;
    edims[1] = wavelengths.size();
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, edims, wavelengths.data(), 0);

    if(element)
        Mat_VarSetStructFieldByName(structArray, fieldnames[1], 0, element);

    // -----------------------
    // 2) filter name
    // -----------------------

    Filter filter = run->filter();

    buf = filter.identifier.toLatin1();
    const char* filter_charp = buf.constData();

    edims[0] = 1;
    edims[1] = buf.length();
    element = Mat_VarCreate(NULL,MAT_C_CHAR,MAT_T_INT8,2,edims,(void*)filter_charp,0);

    if(element != NULL)
        Mat_VarSetStructFieldByName(structArray,fieldnames[2],0,element);

    // -----------------------
    // 3) filter transmission values
    // -----------------------

    QVector<double> filter_values;
    for(int i = 0; i < filter.getTransmissions().size(); i++)
        filter_values.push_back(filter.getTransmissions().at(i));

    edims[0] = 1;
    edims[1] = filter_values.size();
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, edims, filter_values.data(), 0);

    if(element)
        Mat_VarSetStructFieldByName(structArray, fieldnames[3], 0, element);

    // --------------------------
    // 4) laser fluence [mJ/mm^2]
    // --------------------------

    double laserFluence[1] = {run->laserFluence()};
    edims[0] = 1;
    edims[1] = 1;
    element = Mat_VarCreate(NULL,MAT_C_DOUBLE,MAT_T_DOUBLE,2,edims,laserFluence,0);

    if(element != NULL)
        Mat_VarSetStructFieldByName(structArray,fieldnames[4],0,element);

    // --------------------------------
    // 5) pmt channel gain voltages [V]
    // --------------------------------

    const int noch(run->getNoChannels(Signal::RAW));

    QVector<double> pmt;
    for(int i = 0;i < noch; i++)
        pmt.append(run->pmtGainVoltage(i+1));

    edims[0] = 1;
    edims[1] = noch;
    element = 0;
    element = Mat_VarCreate(NULL,MAT_C_DOUBLE,MAT_T_DOUBLE,2,edims,pmt.data(),0);

    if(element != NULL)
        Mat_VarSetStructFieldByName(structArray,fieldnames[5],0,element);

#ifdef LIISIM_FULL
    // -----------------------
    // 6) measured pmt channel gain voltages
    // -----------------------

    QVector<double> measured_pmt_gain;
    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
        measured_pmt_gain.push_back(run->pmtReferenceGainVoltage(i+1));

    edims[0] = 1;
    edims[1] = measured_pmt_gain.size();
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, edims, measured_pmt_gain.data(), 0);

    if(element)
        Mat_VarSetStructFieldByName(structArray, fieldnames[6], 0, element);
#endif

    return structArray;
}


/**
 * @brief IOmatlab::getSignalDataMatrix gets a matrix containing all signals (of all channels) of a certain signal type
 * @param stype signal type
 * @param postProc if set to true, the post processed signal data will be used
 * @return structured array in MatIO-format
 * @details This function is used by IOmatlab::getMRunVar()
 */
matvar_t* IOmatlab::getSignalDataMatrix(MRun *run, Signal::SType stype)
{

    bool postProc = true;
    if(stype == Signal::RAW)
        postProc = e_flag_raw_postproc;
    else if(stype == Signal::ABS)
        postProc = e_flag_abs_postproc;

    // initialize array properties
    const int matrixRank = 3;
    const int nSignals = run->sizeAllMpoints();
    const int nChannels = run->getNoChannels(stype);
    const int nSigFields = 6;
    int matrixDim[matrixRank] = { nSignals, nChannels, nSigFields};

    // to avoid type conversion issues also create a size_t version ...
    size_t matrixDim_s[matrixRank] = { matrixDim[0], matrixDim[1], matrixDim[2]};

    // fieldnames of a signal
    const char* sigFieldNames[nSigFields] =
        {"signal_type",
         "channel_id",
         "size",
         "dt",
         "start_time",
         "data"};

    // init array variable
    matvar_t* matrixVar = Mat_VarCreateStruct(
                NULL,
                matrixRank,
                matrixDim_s,
                sigFieldNames,
                nSigFields);

    if(matrixVar == NULL)
    {
        qDebug() << msgprfx << "getSignalDataMatrix: failed to create signal data array!";
        return 0;
    }

    // iteratre through all channels and signal indices
    for(int i_channel = 0; i_channel < nChannels; i_channel++)
    {

        for(int i_signal = 0; i_signal < nSignals; i_signal++)
        {

            Signal curSignal;

            // get signal from mrun dependent on channel-id,
            // signal-type and pre-/postprocessing option
            if(postProc)
                curSignal = run->getPost( i_signal )->getSignal( i_channel+1, stype );
            else
                curSignal = run->getPre( i_signal )->getSignal( i_channel+1, stype );

            // create all field variables for signal
            for(int i_field = 0; i_field < nSigFields; i_field++)
            {
                matvar_t* signalFieldVar = getSignalFieldVar(curSignal, i_field);

                if(signalFieldVar == 0)
                    continue;

                // get linear matrix index from 3-dimensional subscript
                int subscriptIndex[matrixRank] = { i_signal+1, i_channel+1, 1};
                int linearIndex = Mat_CalcSingleSubscript( matrixRank, matrixDim, subscriptIndex);

                // to avoid type conversion issues use size_t here ...
                size_t linearIndex_s = linearIndex;

                // store signal field variable to data matrix variable
                Mat_VarSetStructFieldByIndex(
                            matrixVar,
                            i_field,
                            linearIndex_s,
                            signalFieldVar);
            }
        }
    }
    return matrixVar;
}


/**
 * @brief IOmatlab::getSignalFieldVar gets a MatIO-varaiable of certain field of a Signal
 * @param s input signal
 * @param fieldIndex index of field
 * @return variable in MatIO-format
 */
matvar_t* IOmatlab::getSignalFieldVar(const Signal &s,int fieldIndex)
{
    matvar_t *element = 0;
    size_t edims[2] = {1,1};

    // ------------------
    // 1) signal type
    // ------------------

    if(fieldIndex == 0)
    {

        QString signal_type = Signal::stypeToString(s.type );
        QByteArray buf = signal_type.toLatin1();
        const char* signal_type_charp = buf.constData();

        edims[0] = 1;
        edims[1] = signal_type.length();
        element = Mat_VarCreate(NULL,MAT_C_CHAR,MAT_T_INT8,2,edims,(void*)signal_type_charp,0);

        if(element == NULL)
        {
            qDebug() << msgprfx << " failed to create signal data field-variable!";
            return 0;
        }
        return element;
    }


    // ------------------
    // 2) channel id
    // ------------------

    if(fieldIndex == 1)
    {
        int chid[1] = {s.channelID};
        edims[0] = 1;
        edims[1] = 1;
        element = Mat_VarCreate(NULL,MAT_C_INT32,MAT_T_INT32,2,edims,chid,0);
        return element;
    }


    // ------------------
    // 3) data size
    // ------------------

    if(fieldIndex == 2)
    {
        int dataSize[1] = {s.data.size()};
        edims[0] = 1;
        edims[1] = 1;
        element = Mat_VarCreate(NULL,MAT_C_INT32,MAT_T_INT32,2,edims,dataSize,0);
        return element;
    }


    // ----------------------
    // 4) delta time
    // ----------------------

    if(fieldIndex == 3)
    {
        double deltaTime[1] = {s.dt};
        edims[0] = 1;
        edims[1] = 1;
        element = Mat_VarCreate(NULL,MAT_C_DOUBLE,MAT_T_DOUBLE,2,edims,deltaTime,0);
        return element;
    }


    // ----------------------
    // 5) start time
    // ----------------------

    if(fieldIndex == 4)
    {
        double startTime[1] = {s.start_time};
        edims[0] = 1;
        edims[1] = 1;
        element = Mat_VarCreate(NULL,MAT_C_DOUBLE,MAT_T_DOUBLE,2,edims,startTime,0);
        return element;
    }


    // ----------------------
    // 6) data vector
    // ----------------------

    if(fieldIndex == 5)
    {
        edims[0] = 1;
        edims[1] = s.data.size();
        element = Mat_VarCreate(NULL,MAT_C_DOUBLE,MAT_T_DOUBLE,2,edims, (void*)s.data.data(), 0);
        return element;
    }

    // invalid field number -> return 0
    return 0;

}


matvar_t* IOmatlab::getSignalTypeStruct(MRun *run, Signal::SType stype, bool unprocessed, bool processed, bool stdev)
{
    matvar_t* structArray = nullptr;
    size_t structDims[2] = {1, 1};

    const int nfields = 2;

    const char* fieldnames[nfields] = {
        "unprocessed",
        "processed"
    };

    structArray = Mat_VarCreateStruct("raw", 2, structDims, fieldnames, nfields);

    if(!structArray)
        return nullptr;

    if(stype == Signal::RAW || stype == Signal::ABS)
    {
        if(unprocessed)
        {
            matvar_t *cell_array, *cell_element;
            size_t dims[2] = {10, 1};

            dims[0] = run->sizeAllMpoints();
            dims[1] = run->getNoChannels(stype);
            cell_array = Mat_VarCreate("unprocessed", MAT_C_CELL, MAT_T_CELL, 2, dims, NULL, 0);
            if(NULL == cell_array)
            {
                //TODO!!!
                qDebug() << "Error creating variable for 'a'";
            }

            int index = 0;

            for(int i = 1; i <= run->getNoChannels(stype); i++)
            {
                for(int j = 0; j < run->sizeAllMpoints(); j++)
                {
                    cell_element = getSignalStruct(run, stype, false, j, i, stdev);
                    if(NULL == cell_element)
                    {
                        //TODO!!!
                        qDebug() << "Error creating cell element variable";
                        Mat_VarFree(cell_array);
                    }
                    Mat_VarSetCell(cell_array, index, cell_element);

                    index++;
                }
            }

            Mat_VarSetStructFieldByName(structArray, fieldnames[0], 0, cell_array);
        }

        if(processed)
        {
            matvar_t *cell_array, *cell_element;
            size_t dims[2] = {1, 1};

            if(run->getProcessingChain(stype)->containsActiveMSA())
                dims[0] = 1;
            else
                dims[0] = run->sizeAllMpoints();

            dims[1] = run->getNoChannels(stype);
            cell_array = Mat_VarCreate("processed", MAT_C_CELL, MAT_T_CELL, 2, dims, NULL, 0);
            if(NULL == cell_array)
            {
                //TODO!!!
                qDebug() << "Error creating variable for 'a'";
            }

            int index = 0;

            for(int i = 1; i <= run->getNoChannels(stype); i++)
            {
                for(int j = 0; j < dims[0]; j++)
                {
                    cell_element = getSignalStruct(run, stype, true, j, i, stdev);
                    if(NULL == cell_element)
                    {
                        //TODO!!!
                        qDebug() << "Error creating cell element variable";
                        Mat_VarFree(cell_array);
                    }
                    Mat_VarSetCell(cell_array, index, cell_element);

                    index++;
                }
            }

            Mat_VarSetStructFieldByName(structArray, fieldnames[1], 0, cell_array);
        }
    }
    else if(stype == Signal::TEMPERATURE)
    {
        if(unprocessed)
        {
            bool msa = true;

            TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
            for(int i = 0; i < run->getNoChannels(Signal::TEMPERATURE); i++)
            {
                TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(i));
                if(tc && !run->getProcessingChain(tc->getInputSignalType())->containsActiveMSA())
                    msa = false;
            }

            matvar_t *cell_array, *cell_element;
            size_t dims[2] = {1, 1};

            if(msa)
                dims[0] = 1;
            else
                dims[0] = run->sizeAllMpoints();

            dims[1] = run->getNoChannels(Signal::TEMPERATURE);
            cell_array = Mat_VarCreate("unprocessed", MAT_C_CELL, MAT_T_CELL, 2, dims, NULL, 0);

            if(!cell_array)
            {
                //TODO!!!
                qDebug() << "Error creating cell structure for unprocessed temperature channels";
            }

            int index = 0;

            for(int i = 0; i < run->getNoChannels(Signal::TEMPERATURE); i++)
            {
                for(int j = 0; j < dims[0]; j++)
                {
                    cell_element = getTempSignalStruct(run, j, i, false, true);
                    if(NULL == cell_element)
                    {
                        //TODO!!!
                        qDebug() << "Error creating cell element variable";
                        Mat_VarFree(cell_array);
                    }

                    Mat_VarSetCell(cell_array, index, cell_element);

                    index++;
                }
            }

            Mat_VarSetStructFieldByName(structArray, fieldnames[0], 0, cell_array);
        }

        if(processed)
        {
            TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
            if(tpchain->noPlugs() > 1 && tpchain->getPlug(tpchain->noPlugs()-1)->getName() != "Dummy")
            {
                matvar_t *cell_array, *cell_element;
                size_t dims[2] = {1, 1};

                dims[0] = run->sizeAllMpoints();
                dims[1] = run->getNoChannels(Signal::TEMPERATURE);
                cell_array = Mat_VarCreate("unprocessed", MAT_C_CELL, MAT_T_CELL, 2, dims, NULL, 0);

                if(!cell_array)
                {
                    //TODO!!!
                    qDebug() << "Error creating cell structure for unprocessed temperature channels";
                }

                int index = 0;

                for(int i = 0; i < run->getNoChannels(Signal::TEMPERATURE); i++)
                {
                    for(int j = 0; j < run->sizeAllMpoints(); j++)
                    {
                        cell_element = getTempSignalStruct(run, j, i, true, true);
                        if(NULL == cell_element)
                        {
                            //TODO!!!
                            qDebug() << "Error creating cell element variable";
                            Mat_VarFree(cell_array);
                        }

                        Mat_VarSetCell(cell_array, index, cell_element);

                        index++;
                    }
                }

                Mat_VarSetStructFieldByName(structArray, fieldnames[1], 0, cell_array);
            }
        }
    }

    return structArray;
}


matvar_t* IOmatlab::getSignalStruct(MRun *run, Signal::SType stype, bool processed, int signalID, int channelID, bool stdev)
{
    Signal signal;

    if(processed)
        signal = run->getPost(signalID)->getSignal(channelID, stype);
    else
        signal = run->getPre(signalID)->getSignal(channelID, stype);

    if(signal.data.isEmpty())
    {
        qDebug() << "---> Matlab export error: signal data empty";
        return nullptr;
    }

    matvar_t* struct_array = nullptr;
    size_t struct_dims[2] = {1, 1};

    const int fields = 7;
    const char* fieldnames[fields] = {
        "signal_type",
        "channel_id",
        "size",
        "dt",
        "start_time",
        "data",
        "stdev"
    };

    struct_array = Mat_VarCreateStruct(NULL, 2, struct_dims, fieldnames, fields);

    if(!struct_array)
    {
        //error handling goes here
        return nullptr;
    }

    matvar_t* element;

    //write signal type

    QString string_stype;

    switch(stype)
    {
    case Signal::RAW: string_stype = QString("raw"); break;
    case Signal::ABS: string_stype = QString("absolute"); break;
    case Signal::TEMPERATURE: string_stype = QString("temperature"); break;
    }

    QByteArray buffer = string_stype.toLatin1();
    const char* stype_char = buffer.constData();

    struct_dims[0] = 1;
    struct_dims[1] = string_stype.length();
    element = Mat_VarCreate(NULL, MAT_C_CHAR, MAT_T_INT8, 2, struct_dims, (void*)stype_char, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[0], 0, element);

    //write channel id

    int32_t int_element[1] = { channelID };

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, struct_dims, (void*)int_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[1], 0, element);

    //write size

    int_element[0] = signal.data.size();

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, struct_dims, (void*)int_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[2], 0, element);

    //write dt

    double double_element[1] = { signal.dt };

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, (void*)double_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[3], 0, element);

    //write start time

    double_element[0] = signal.start_time;

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, (void*)double_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[4], 0, element);

    //write data

    struct_dims[0] = signal.data.size();
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, signal.data.data(), 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[5], 0, element);

    //write standard deviation

    if(stdev && signal.stdev.size() > 0)
    {
        struct_dims[0] = signal.stdev.size();
        struct_dims[1] = 1;
        element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, signal.stdev.data(), 0);

        if(element)
            Mat_VarSetStructFieldByName(struct_array, fieldnames[6], 0, element);
    }

    return struct_array;
}


matvar_t* IOmatlab::getTempSignalStruct(MRun *run, int signalID, int channelID, bool processed, bool stdev)
{
    Signal signal;
    int tcChannelID = channelID;

    if(processed)
    {
        TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
        TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(channelID));
        tcChannelID = tc->temperatureChannelID();

        if(tpchain->getPlug(tpchain->noPlugs()-1)->getName() == "Temperature Calculator")
            qDebug() << "Error: last plugin is a temperature calculator";

        signal = tpchain->getPlug(tpchain->noPlugs()-1)->processedSignal(signalID, tcChannelID);
    }
    else
    {
        TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
        TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(channelID));
        signal = tc->processedSignal(signalID, tc->temperatureChannelID());
        tcChannelID = tc->temperatureChannelID();
    }

    matvar_t* struct_array = nullptr;
    size_t struct_dims[2] = {1, 1};

    const int fields = 8;
    const char* fieldnames[fields] = {
        "signal_type",
        "channel_id",
        "temperature_metadata",
        "size",
        "dt",
        "start_time",
        "data",
        "stdev"
    };

    struct_array = Mat_VarCreateStruct(NULL, 2, struct_dims, fieldnames, fields);

    if(!struct_array)
    {
        qDebug() << "Error: getTempSignalStruct() - creating base struct";
        return nullptr;
    }
    matvar_t* element;

    //write signal type

    QByteArray buffer = QString("temperature").toLatin1();
    const char* stype_char = buffer.constData();

    struct_dims[0] = 1;
    struct_dims[1] = buffer.length();
    element = Mat_VarCreate(NULL, MAT_C_CHAR, MAT_T_INT8, 2, struct_dims, (void*)stype_char, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[0], 0, element);

    //write channel id

    int32_t int_element[1] = { tcChannelID };

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, struct_dims, (void*)int_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[1], 0, element);

    //write temperature meta data

    struct_dims[0] = 1;
    struct_dims[1] = 1;

    const int metafields = 3;
    const char* metafieldnames[metafields] = {
        "method",
        "channels",
        "material"
    };

    matvar_t* metadata = Mat_VarCreateStruct(NULL, 2, struct_dims, metafieldnames, metafields);

    if(metadata)
    {
        TempCalcMetadata meta = run->tempMetadata.value(tcChannelID);

        buffer = meta.method.toLatin1();
        const char* tempCalcMethod = buffer.constData();

        struct_dims[0] = 1;
        struct_dims[1] = buffer.length();
        element = Mat_VarCreate(NULL, MAT_C_CHAR, MAT_T_INT8, 2, struct_dims, (void*)tempCalcMethod, 0);

        if(element)
            Mat_VarSetStructFieldByName(metadata, metafieldnames[0], 0, element);

        QVector<int32_t> channels;

        if(meta.method == "Two Color")
        {
            channels.push_back(meta.channelID1);
            channels.push_back(meta.channelID2);
        }
        else if(meta.method == "Spectrum")
        {
            for(int i = 0; i < meta.activeChannels.size(); i++)
            {
                if(meta.activeChannels.at(i))
                    channels.push_back(i+1);
            }
        }

        struct_dims[0] = 1;
        struct_dims[1] = channels.size();
        element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, struct_dims, channels.data(), 0);

        if(element)
            Mat_VarSetStructFieldByName(metadata, metafieldnames[1], 0, element);

        buffer = meta.material.toLatin1();
        const char* material = buffer.constData();

        struct_dims[0] = 1;
        struct_dims[1] = buffer.length();
        element = Mat_VarCreate(NULL, MAT_C_CHAR, MAT_T_INT8, 2, struct_dims, (void*)material, 0);

        if(element)
            Mat_VarSetStructFieldByName(metadata, metafieldnames[2], 0, element);

        Mat_VarSetStructFieldByName(struct_array, fieldnames[2], 0, metadata);
    }

    //write size

    int_element[0] = signal.data.size();

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_INT32, MAT_T_INT32, 2, struct_dims, (void*)int_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[3], 0, element);

    //write dt

    double double_element[1] = { signal.dt };

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, (void*)double_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[4], 0, element);

    //write start_time

    double_element[0] = { signal.start_time };

    struct_dims[0] = 1;
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, (void*)double_element, 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[5], 0, element);

    //write data

    struct_dims[0] = signal.data.size();
    struct_dims[1] = 1;
    element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, signal.data.data(), 0);

    if(element)
        Mat_VarSetStructFieldByName(struct_array, fieldnames[6], 0, element);

    //write standard deviation

    if(stdev && signal.data.size() > 0)
    {
        struct_dims[0] = signal.stdev.size();
        struct_dims[1] = 1;
        element = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, struct_dims, signal.stdev.data(), 0);

        if(element)
            Mat_VarSetStructFieldByName(struct_array, fieldnames[7], 0, element);
    }

    return struct_array;
}
