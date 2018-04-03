#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include <QString>

/**
 * @brief manages loading .html help files
 * @ingroup GUI-Utilities
 * @details This class is used to embed simple .html help text within
 * the program.
 */
class HelpManager
{
public:
    HelpManager();

    static QString getHelpFileName(QString tag);
    static QString getHelpHTML(QString tag);
    static void showMessageBox(QString tag);

};

#endif // HELPMANAGER_H
