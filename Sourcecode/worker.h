#include "configmanager.h"

#include <QObject>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QFile>
#include <QColor>
#include <QElapsedTimer>
#include <QDir>
#include <qregularexpression.h>



class HashWorker : public QObject {
    Q_OBJECT
public:
    explicit HashWorker( const QString &path, const QString &sFilter )
        : m_path(path), m_filter(sFilter), bAbortSignal(false) {}

    // Cancel function
    void __abort() { bAbortSignal = true; }

public slots:

    ////////////////////////////////////////////////
    ///
    ///   PROCESS
    ///
    ////////////////////////////////////////////////

    void process() {

        CMI.iInspectCounter++;      // Count along

        int           iSuccessCount = 0, iErrorCount = 0, iSkippedCount = 0;
        QStringList   slNameFilters;
        QString       sLastDirectory;
        QElapsedTimer timerDuration;    timerDuration.start();
        QElapsedTimer lastUpdateTimer;  lastUpdateTimer.start();

        for( const QString &sExt : m_filter.split(';', Qt::SkipEmptyParts) )  {

            slNameFilters << "*." + sExt.trimmed();

        }


        ///////////////////////////
        /// Prepare the filter
        QStringList lFilterList = CMI.sExcludePat.split( '\n', Qt::SkipEmptyParts );

        for( QString &sLine : lFilterList )  {

            sLine = sLine.trimmed();

            // Backslashes to forward slashes (for Linux/Qt standard)
            sLine.replace( '\\', '/' );

            // Lowercase (so the filter always works)
            sLine = sLine.toLower();

            // Consistency: Folder patterns should ideally not include a leading slash
            if ( sLine.startsWith('/') ) sLine.remove( 0, 1 );

        }



        ///////////////////////////
        /// Determine hash algorithms

        QList<QCryptographicHash::Algorithm> xAlgos      = { };
        QString                              sHashAlgos  = "";
        QString                              sHashFormat = "Undef";

        if ( CMI.bHashF_MD5     == true )  {  xAlgos.append( QCryptographicHash::Md5      );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("MD5");       }
        if ( CMI.bHashF_SHA1    == true )  {  xAlgos.append( QCryptographicHash::Sha1     );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("SHA1");      }
        if ( CMI.bHashF_SHA256  == true )  {  xAlgos.append( QCryptographicHash::Sha256   );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("SHA256");    }
        if ( CMI.bHashF_SHA512  == true )  {  xAlgos.append( QCryptographicHash::Sha512   );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("SHA512");    }
        if ( CMI.bHashF_SHA3256 == true )  {  xAlgos.append( QCryptographicHash::Sha3_256 );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("SHA3-256");  }
        if ( CMI.bHashF_SHA3512 == true )  {  xAlgos.append( QCryptographicHash::Sha3_512 );  sHashAlgos += (sHashAlgos.isEmpty() ? "" : "  ") + QString("SHA3-512");  }

        if ( xAlgos.isEmpty() )  {  emit messageLogged( "Fatal error in process(), no hash function selected", Qt::red );
                                    emit finished( iSuccessCount, iErrorCount );
                                    return;
                                 }

        switch( CMI.iHashFormat )  {

                case RADIOBUTTON_0 :  sHashFormat = "Ignore case";     break;
                case RADIOBUTTON_1 :  sHashFormat = "Uppercase onyl";  break;
                case RADIOBUTTON_2 :  sHashFormat = "Lowercase onyl";  break;

        }

        ///////////////////////////
        /// Header

        emit messageLogged( QString("Starting inspection of:  %1").arg(m_path), __getSysTXTcolor(), true );

        if ( CMI.sAllowedExts.isEmpty() )  emit messageLogged( "Allowed Extensions:  Empty (All files)", __getSysTXTcolor(), true );
        else  emit messageLogged( QString("Allowed Extensions:  %1").arg(CMI.sAllowedExts), __getSysTXTcolor(), true );

        emit messageLogged( QString("Hash Functions:  %1  (%2)").arg(sHashAlgos).arg(sHashFormat), __getSysTXTcolor(), true );
        emit messageLogged( "-----------------------------------", __getSysTXTcolor() );


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// Processing loop

        QDirIterator::IteratorFlags itFlags = CMI.bIncludeSubf ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
        QDirIterator it( m_path, slNameFilters, QDir::Files | QDir::NoDotAndDotDot, itFlags );

        while( it.hasNext() )  {

            // Abort oder Auto-stop?
            if ( bAbortSignal )  {  emit messageLogged( "Process aborted by user", Qt::red );
                                    break;
                                 }

            if ( CMI.iAutoStopMM > 0 )
            if ( iErrorCount >= CMI.iAutoStopMM )  {  emit messageLogged( QString("Auto-stop hash mismatches (%1)").arg(CMI.iAutoStopMM), Qt::red );
                                                      break;
                                                   }
            if ( CMI.iAutoStopAI > 0 )
            if ( iSuccessCount+iErrorCount >= CMI.iAutoStopAI )  {  emit messageLogged( QString("Auto-stop inspected files (%1)").arg(CMI.iAutoStopAI), Qt::red );
                                                                    break;
                                                                 }

            // Retrieve data
            QFileInfo xFileInfo = it.nextFileInfo();

            // Prevents files without a period or file extension from slipping through
            if ( !m_filter.isEmpty() && xFileInfo.suffix().isEmpty() )  continue;

            // Filter 1 und 2
            if ( __filter1(lFilterList,xFileInfo) == false || __filter2(xFileInfo.completeBaseName()) == false )  {

               iSkippedCount++;

               // Send an update every 100 ms
               if ( lastUpdateTimer.hasExpired(100) )  {

                    emit counterUpdate( iSuccessCount, iErrorCount, iSkippedCount );
                    lastUpdateTimer.restart();

                }
                continue;

            }

            // If desired, display the path information in the ListWidget
            if ( CMI.bShowScanPath == true )  {

                QString sCurrentDir = xFileInfo.absolutePath();
                if ( sCurrentDir != sLastDirectory )  {  sLastDirectory = sCurrentDir;
                                                         emit messageLogged( QString("Scanning:  %1").arg(sCurrentDir), __getSysTXTcolor() );
                                                      }

            }

            // Get more information about the file
            QString sFilePath = xFileInfo.absoluteFilePath();
            QString sFileName = xFileInfo.fileName();
            QString sPurePath = xFileInfo.absolutePath();        // Pfad für den Tooltip

            // Create a draft entry
            emit fileInProgress( sFileName );

            // Compute hashes in chunks (to allow for immediate termination)
            QFile file( sFilePath );

            if ( file.open(QIODevice::ReadOnly) )  {

                // Create a list for the hash objects
                QList<QCryptographicHash*> hashObjects;
                for( auto xAlgo : xAlgos )  hashObjects.append( new QCryptographicHash(xAlgo) );

                qint64 qiFileSize          = file.size();     // Total size of the current file
                qint64 qiBytesProcessed    = 0;              // Counter for bytes read
                int    iLastEmittedPercent = -1;

                bool bShowProgress = (qiFileSize >= 100 * 1024 * 1024);
                if ( bShowProgress == true )  emit fileHashProgress( 0 );

                while( !file.atEnd() && !bAbortSignal )  {

                    QByteArray aBlock = file.read( 1024 * 1024 );

                    // Distribute the block to ALL Hasher
                    for( auto* h : hashObjects )  h->addData( aBlock );
                    qiBytesProcessed += aBlock.size();

                    // Calculate progress as a percentage
                    if ( bShowProgress )  {

                            int iCurrentPercent = static_cast<int>((qiBytesProcessed * 100) / qiFileSize);

                            // Send only when a new 10% milestone has been reached
                            int iStep = (iCurrentPercent / 10) * 10;
                            if ( iStep > iLastEmittedPercent )  {

                                emit fileHashProgress( iStep );
                                iLastEmittedPercent = iStep;

                            }
                    }
                }

                if ( !bAbortSignal )  {

                        bool        bFound = false;
                        QString     sFinalHash = "undef";
                        QStringList slAllHashes;    // List of hashes for error handling

                        // Go through all the calculated hashes
                        for( int i = 0; i < hashObjects.size(); ++i )  {

                            QString sHashResult = hashObjects[i]->result().toHex();

                            // Formatting based on the radio buttons
                            Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;

                            switch( CMI.iHashFormat )  {

                                case RADIOBUTTON_1  :  sHashResult = sHashResult.toUpper(); sensitivity = Qt::CaseSensitive;  break;
                                case RADIOBUTTON_2  :  sHashResult = sHashResult.toLower(); sensitivity = Qt::CaseSensitive;  break;
                                default             :  sensitivity = Qt::CaseInsensitive;                                     break;

                            }

                            // If nothing is found, collect all hashes
                            QString sAlgoName = __getHashAlgoName( xAlgos[i] );
                            slAllHashes << QString( "%1 :  %2" ).arg(sAlgoName, sHashResult);

                            // Check: If this hash is in the name -> Success!
                            if ( sFileName.contains(sHashResult, sensitivity) )  {

                                bFound     = true;
                                sFinalHash = sHashResult;
                                break;  // Stop the search—we've found a match

                            }

                        }  // for

                        // If no match is found, separate all collected hashes with semicolons
                        if ( !bFound )  {  sFinalHash = slAllHashes.join( ";" );
                                           iErrorCount++;
                                        }
                        else  iSuccessCount++;

                        // Change the color of the item and send the result
                        emit hashResultReady( bFound, iSuccessCount, iErrorCount, iSkippedCount, sPurePath, sFinalHash );

                }   // if !m_abort

                // Free up storage space
                qDeleteAll( hashObjects );
                hashObjects.clear();
                file.close();

            }   // if ( file.open

            else  {  iErrorCount++;
                     emit messageLogged( QString("Can' read file: %1").arg(sFileName), Qt::red );
                  }

        }   // while

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        ///////////////////////////
        /// Reporting of results

        qint64  qiMsec = timerDuration.elapsed();
        QString sFormattedTime;

        if ( qiMsec < 1000 )  sFormattedTime = "< 1 second";
        else {  qint64 qiTotalSeconds = qiMsec / 1000;
                int iH = qiTotalSeconds / 3600;
                int iM = (qiTotalSeconds % 3600) / 60;
                int iS = qiTotalSeconds % 60;

                if ( iH == 0 && iM == 0 ) sFormattedTime = QString( "%1 seconds" ).arg(iS);
                else sFormattedTime = QString("%1:%2:%3").arg(iH,2,10,QChar('0')).arg(iM,2,10,QChar('0')).arg(iS,2,10,QChar('0'));
        }

        if ( iSuccessCount + iErrorCount == 0 && !bAbortSignal )  emit messageLogged( "Sorry, no matches", Qt::red );
        emit messageLogged( "-------------------------", __getSysTXTcolor() );
        emit messageLogged( QString("Inspection runtime: %1").arg(sFormattedTime), __getSysTXTcolor() );
        emit messageLogged( QString("Hash found: %1, not found: %2, skippend: %3").arg(iSuccessCount).arg(iErrorCount).arg(iSkippedCount), __getSysTXTcolor() );

        emit finished( iSuccessCount, iErrorCount );
    }

signals:

    void messageLogged( const QString, const QColor, const bool bIcon = false );
    void fileInProgress( const QString );
    void fileHashProgress( const int );
    void counterUpdate( const int, const int, const int );
    void hashResultReady( const bool, const int, const int, const int, const QString, const QString );

    void fileProcessed( QString, bool );
    void finished( int, int );


private:

    QString m_path;
    QString m_filter;
    std::atomic<bool> bAbortSignal;

    ////////////////////////////////////////////////
    ///
    ///   FILTER
    ///
    ////////////////////////////////////////////////

    // Filter 1: This feature uses lFilterList to filter out unwanted files and folders
    bool __filter1( const QStringList &lFilterList, const QFileInfo xFileInfo )  {

            // Standardize path
            QString sPath = QDir::fromNativeSeparators( xFileInfo.absoluteFilePath().toLower() );

            // Important: The folder path must end with "/" in order to match patterns like "system/"
            if ( xFileInfo.isDir() && !sPath.endsWith('/') ) sPath += '/';

            QString sName = xFileInfo.fileName().toLower();

            for( const QString &sPattern : lFilterList )  {

                if ( sPattern.isEmpty() ) continue;

                // We check whether the pattern contains slashes.
                // If so, it is a path pattern (e.g., "*/.trash-*/" or "system/")
                bool isPathPattern = sPattern.contains( '/' );

                if ( isPathPattern )  {

                            // QDir::match also works on paths!
                            // We check the entire sPath against the pattern.
                            if ( QDir::match(sPattern, sPath) ) return false;

                            // Just to be safe: If the pattern doesn't contain a wildcard but is part of the path
                            if ( sPath.contains(sPattern) ) return false;

                }  else  {  // File logic: Simple match against the file name alone
                            if ( QDir::match(sPattern, sName) ) return false;
                }
            }

            return true;

    }   // __filter


    // Filter 2: Checks whether a filename can contain a hash at all
    bool __filter2( const QString sBaseFileName )  {

        // We're building a pattern that searches for specific lengths.
        // \b ensures that the hash is positioned at a word boundary (space, period, start/end).
        // {32} etc. defines the exact number of characters.

        QStringList slPatterns;
        if ( CMI.bHashF_MD5                          )  slPatterns << "[0-9a-fA-F]{32}";
        if ( CMI.bHashF_SHA1                         )  slPatterns << "[0-9a-fA-F]{40}";
        if ( CMI.bHashF_SHA256 || CMI.bHashF_SHA3256 )  slPatterns << "[0-9a-fA-F]{64}";
        if ( CMI.bHashF_SHA512 || CMI.bHashF_SHA3512 )  slPatterns << "[0-9a-fA-F]{128}";

        if ( slPatterns.isEmpty() ) return false;

        // Wir verbinden die Muster mit einem "ODER" (|) und setzen Wortgrenzen (\b) davor und danach.
        // Das \b erkennt Leerzeichen, Punkte und String-Grenzen automatisch korrekt.
        QString sFinalPattern = "\\b(" + slPatterns.join( "|" ) + ")\\b";

        static QRegularExpression re;
        re.setPattern( sFinalPattern );

        return re.match( sBaseFileName ).hasMatch();
    }

    // Help function for process()
    QString __getHashAlgoName( QCryptographicHash::Algorithm algo )  {

        switch( algo ) {

            case QCryptographicHash::Md5       :  return "MD5";
            case QCryptographicHash::Sha1      :  return "SHA1";
            case QCryptographicHash::Sha256    :  return "SHA256";
            case QCryptographicHash::Sha512    :  return "SHA512";
            case QCryptographicHash::Sha3_256  :  return "SHA3-256";
            case QCryptographicHash::Sha3_512  :  return "SHA3-512";

            default : return "Unknown";

        }
    }

};

