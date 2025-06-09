#include "filesystem_utils.hpp"

#include <qdir.h>
#include <qprocess.h>

namespace firelight::gui {

bool FilesystemUtils::showInFilesystem(const QString &path) {
  const QFileInfo fileInfo(path);
#ifdef Q_OS_WIN
  QStringList param;
  if (!fileInfo.isDir()) {
    param += QLatin1String("/select,");
  }

  param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
  QProcess::startDetached("explorer.exe", param);
#endif

#ifdef Q_OS_MACOS
  QStringList scriptArgs;
  scriptArgs << QLatin1String("-e")
             << QString::fromLatin1(
                    "tell application \"Finder\" to reveal POSIX file \"%1\"")
                    .arg(fileInfo.canonicalFilePath());
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
  scriptArgs.clear();
  scriptArgs << QLatin1String("-e")
             << QLatin1String("tell application \"Finder\" to activate");
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#endif
  // else {
  //   // we cannot select a file here, because no file browser really supports
  //   // it...
  //   const QString folder =
  //       fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
  //   const QString app = UnixUtils::fileBrowser(ICore::settings());
  //   QProcess browserProc;
  //   const QString browserArgs =
  //       UnixUtils::substituteFileBrowserParameters(app, folder);
  //   bool success = browserProc.startDetached(browserArgs);
  //   const QString error =
  //       QString::fromLocal8Bit(browserProc.readAllStandardError());
  //   success = success && error.isEmpty();
  //   if (!success)
  //     showGraphicalShellError(parent, app, error);
  return true;
}
} // namespace firelight::gui