#ifndef MEMUSAGEMONITOR_H
#define MEMUSAGEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <stdint.h>


/**
 * @brief The MemUsageMonitor class provides usage statistics
 * about virtual and physical memory. It also provides a mechanism
 * for the estimation of the memory usage for the next signal processing
 * task.
 */
class MemUsageMonitor : public QObject
{
    Q_OBJECT

public:
    MemUsageMonitor();
    ~MemUsageMonitor();


    QString msg_prefix;


    /// total virtual memory available [B]
    inline uint64_t virtualMemory(){return v_mem;}

    /// virtual memory used by all processes [B]
    inline uint64_t virtualMemoryUsed(){return v_mem_used;}

    /// virtual memory used by LIISeim [B]
    inline uint64_t virtualMemoryUsedByLIISim(){return v_mem_liisim;}

    /// total physical memory available [B]
    inline uint64_t physicalMemory(){return mem;}

    /// physical memory used by all processes [B]
    inline uint64_t physicalMemoryUsed(){return mem_used;}

    /// physical memory used by LIISim [B]
    inline uint64_t physicalMemoryUsedByLIISim(){return mem_liisim;}

    /// max amount of memory which LIISim is allowed to use [B]
    inline uint64_t physicalMemoryAllowed(){return mem_allowed;}

    static double toKB(uint64_t bytes);
    static double toMB(uint64_t bytes);
    static double toGB(uint64_t bytes);

    uint64_t runDataEstimate();
    uint64_t estimateMemUsageForNextProcessingTask();

    bool checkIfProcessingIsAllowed();

    /// check if LIISim uses 32-bit adress space
    bool is32bit(){return m_is32bit;}

private:


    /// total virtual memory available
    uint64_t v_mem;

    /// max allowed virtual memory
    uint64_t v_mem_allowed;

    /// virtual memory used by all processes
    uint64_t v_mem_used;

    /// virtual memory used by current process = LIISim
    uint64_t v_mem_liisim;

    /// total physical memory available
    uint64_t mem;

    /// max allowed physical memory
    uint64_t mem_allowed;

    /// pysical memory used by all processes
    uint64_t mem_used;

    /// physical memory used by current process = LIISim
    uint64_t mem_liisim;


    bool m_is32bit;

    /// timer, responsible for updating memory infos periodically
    QTimer timer;

    /// Update interval of timer in ms
    int updateInterval;

signals:

    /**
     * @brief infosUpdated This signal is emitted when
     * the memory informations have been updated
     */
    void infosUpdated();

public slots:

    bool updateMemInfo();

};

#endif // MEMUSAGEMONITOR_H
