/**
 * @file pstack_text_utils.cpp
 */

#include "pstack_text_utils.hpp"

#include <QRegularExpression>

namespace scope::desktop
{

std::vector<PstackThreadBlock> splitPstackThreads(const QString& text)
{
    std::vector<PstackThreadBlock> blocks;

    if (text.isEmpty())
    {
        return blocks;
    }

    const QStringList lines = text.split(QLatin1Char('\n'));
    PstackThreadBlock current;
    QRegularExpression threadHeader(QStringLiteral("^(?:Thread (\\d+)|TID (\\d+):)"));

    for (const QString& line : lines)
    {
        const QRegularExpressionMatch match = threadHeader.match(line);

        if (match.hasMatch())
        {
            if (!current.text.isEmpty())
            {
                blocks.push_back(current);
            }

            current = PstackThreadBlock{};
            current.id = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
            current.text = line;

            continue;
        }

        if (current.text.isEmpty())
        {
            current.id = QStringLiteral("0");
            current.text = line;
        }
        else
        {
            current.text += QLatin1Char('\n');
            current.text += line;
        }
    }

    if (!current.text.isEmpty())
    {
        blocks.push_back(current);
    }

    return blocks;
}

} // namespace scope::desktop
