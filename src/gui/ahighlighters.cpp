#include "ahighlighters.h"

#include <QDebug>

AHighlighterScriptWindow::AHighlighterScriptWindow(QTextDocument *parent)
   : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::blue);
    //keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bbreak\\b" << "\\bcatch\\b" << "\\bcontinue\\b" << "\\b.length\\b" << "\\barguments\\b"
                    << "\\bdo\\b" << "\\bwhile\\b" << "\\bfor\\b"
                    << "\\bin\\b" << "\\bfunction\\b" << "\\bif\\b"
                    << "\\belse\\b" << "\\breturn\\b" << "\\bswitch\\b"
                    << "\\bthrow\\b" << "\\btry\\b" << "\\bvar\\b" << "\\bpush\\b" << "\\btypeof\\b"
                    << "\\Math.\\b" << "\\Array.\\b" << "\\String.\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

/*
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);
*/

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);

    quotationFormat.setForeground(Qt::darkGreen);
    //QRegularExpression rx("\".*\"");
    QRegularExpression rx("((?<![\\\\])['\"])((?:.(?!(?<![\\\\])\\1))*.?)\\1");
    //qDebug() << "----------------------"<< rx.isValid();
    //rx.setMinimal(true); //fixes the problem with "xdsfdsfds" +variable+ "dsfdsfdsf"
    rule.pattern = rx;
    rule.format = quotationFormat;
    highlightingRules.append(rule);

//    charFormat.setForeground(Qt::darkGreen);
//    rule.pattern = QRegularExpression("'.*'");
//    rule.format = charFormat;
//    highlightingRules.append(rule);

/*
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
*/

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void AHighlighterScriptWindow::setCustomCommands(QStringList functions, QStringList constants)
{
    QVector<HighlightingRule> hr;

    HighlightingRule rule;

    QColor color = Qt::darkCyan;
    customKeywordFormat.setForeground(color.darker(110));
    //customKeywordFormat.setForeground(Qt::darkCyan);
    //customKeywordFormat.setFontWeight(QFont::Bold);
    //customKeywordFormat.setFontItalic(true);

    color = Qt::darkMagenta;
    unitFormat.setForeground(color);

    QSet<QString> units;
    for (const QString& pattern : functions)
    {
        rule.pattern = QRegularExpression("\\b"+pattern+"(?=\\()");
        rule.format = customKeywordFormat;
        hr.append(rule);

        QStringList f = pattern.split(".", QString::SkipEmptyParts);
        if (f.size() > 1 && !f.first().isEmpty()) units << f.first();
    }
    for (const QString& pattern : units)
    {
        rule.pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.format = unitFormat;
        hr.append(rule);
    }
    for (const QString &pattern : constants)
    {
        rule.pattern = QRegularExpression("\\b"+pattern+"\\b(?![\\(\\{\\[])");
        rule.format = customKeywordFormat;
        hr.append(rule);
    }

    highlightingRules = hr + highlightingRules; //so e.g. comments and quatation rule have higher priority
}

void AHighlighterScriptWindow::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
      {
//             QRegularExpression expression(rule.pattern);
//             int index = expression.indexIn(text);
//             while (index >= 0) {
//                 int length = expression.matchedLength();
//                 setFormat(index, length, rule.format);
//                 index = expression.indexIn(text, index + length);
//
//             }
        /*
               QRegularExpression exp(rule.pattern);
               QRegularExpressionMatchIterator matches = exp.globalMatch(text);
               while (matches.hasNext())
               {
                    QRegularExpressionMatch match = matches.next();
                    setFormat(match.capturedStart(), match.capturedLength(), rule.format);
               }
         */
               QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
               while (matchIterator.hasNext())
               {
                    QRegularExpressionMatch match = matchIterator.next();
                    setFormat(match.capturedStart(), match.capturedLength(), rule.format);
               }
    }

    if (bMultilineCommentAllowed)
    {
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
             //    startIndex = commentStartExpression.indexIn(text);
            startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0)
          {
//                 int endIndex = commentEndExpression.indexIn(text, startIndex);
//                 int commentLength;
//                 if (endIndex == -1) {
//                     setCurrentBlockState(1);
//                     commentLength = text.length() - startIndex;
//                 } else {
//                     commentLength = endIndex - startIndex
//                                     + commentEndExpression.matchedLength();
//                 }
//                 setFormat(startIndex, commentLength, multiLineCommentFormat);
//                 startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);

                    QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
                    int endIndex = match.capturedStart();
                    int commentLength = 0;
                    if (endIndex == -1) {
                        setCurrentBlockState(1);
                        commentLength = text.length() - startIndex;
                    } else {
                        commentLength = endIndex - startIndex
                                        + match.capturedLength();
                    }
                    setFormat(startIndex, commentLength, multiLineCommentFormat);
                    startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
           }
    }
}

AHighlighterLrfScript::AHighlighterLrfScript(QTextDocument *parent)
  : AHighlighterScriptWindow(parent)
{
  setFixedVariables();
}

void AHighlighterLrfScript::setFixedVariables()
{
  QStringList variables;
  variables << "r" << "R";

  HighlightingRule rule;

  //customKeywordFormat.setForeground(Qt::darkCyan);
  customKeywordFormat.setForeground(Qt::cyan);
  customKeywordFormat.setFontWeight(QFont::Bold);
//    customKeywordFormat.setFontItalic(true);

  QVector<HighlightingRule> hr;
//  foreach (const QString &pattern, functions)
//    {
//      rule.pattern = QRegExp("\\b"+pattern+"(?=\\()");
//      rule.format = customKeywordFormat;
//      hr.append(rule);
//    }
  foreach (const QString &pattern, variables)
    {
      rule.pattern = QRegularExpression("\\b"+pattern+"\\b(?![\\(\\{\\[])");
      rule.format = customKeywordFormat;
      hr.append(rule);
    }

  highlightingRules = hr + highlightingRules; //so e.g. comments and quatation rule have higher priority
}

AHighlighterPythonScriptWindow::AHighlighterPythonScriptWindow(QTextDocument *parent) :
    AHighlighterScriptWindow(parent)
{
    bMultilineCommentAllowed = false;
    highlightingRules.clear();

    HighlightingRule rule;

    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "and" << "assert" << "break" << "class" << "continue" <<
                       "def" << "del" << "elif" << "else" << "except" <<
                       "exec" << "finally" << "for" << "from" << "global" <<
                       "if" << "import" << "in" << "is" << "lambda" <<
                       "not" << "or" << "pass" << "print" << "raise" <<
                       "return" << "try" << "while" <<
                       "Data" << "Float" << "Int" << "Numeric" << "Oxphys" <<
                       "array" << "close" << "float" << "int" << "input" <<
                       "open" << "range" << "type" << "write" << "zeros";

    foreach (const QString &pattern, keywordPatterns)
    {
        QString pattern1 = QString("\\b") + pattern + "\\b";

        rule.pattern = QRegularExpression(pattern1);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

/*
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);
*/

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);

    quotationFormat.setForeground(Qt::darkGreen);
    //QRegularExpression rx("\".*\"");
    QRegularExpression rx("((?<![\\\\])['\"])((?:.(?!(?<![\\\\])\\1))*.?)\\1");
    //qDebug() << "----------------------"<< rx.isValid();
    //rx.setMinimal(true); //fixes the problem with "xdsfdsfds" +variable+ "dsfdsfdsf"
    rule.pattern = rx;
    rule.format = quotationFormat;
    highlightingRules.append(rule);

//    charFormat.setForeground(Qt::darkGreen);
//    rule.pattern = QRegularExpression("'.*'");
//    rule.format = charFormat;
//    highlightingRules.append(rule);

/*
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
*/

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}
