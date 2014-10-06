/* Webcamoid, webcam capture application.
 * Copyright (C) 2011-2014  Gonzalo Exequiel Pedone
 *
 * Webcamod is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamod is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamod. If not, see <http://www.gnu.org/licenses/>.
 *
 * Email     : hipersayan DOT x AT gmail DOT com
 * Web-Site 1: http://github.com/hipersayanX/Webcamoid
 * Web-Site 2: http://opendesktop.org/content/show.php/Webcamoid?content=144796
 */

#include "warpelement.h"

WarpElement::WarpElement(): QbElement()
{
    this->m_convert = QbElement::create("VCapsConvert");
    this->m_convert->setProperty("caps", "video/x-raw,format=bgra");

    this->resetRipples();
}

float WarpElement::ripples() const
{
    return this->m_ripples;
}

void WarpElement::setRipples(float ripples)
{
    this->m_ripples = ripples;
}

void WarpElement::resetRipples()
{
    this->setRipples(4);
}

QbPacket WarpElement::iStream(const QbPacket &packet)
{
    QbPacket iPacket = this->m_convert->iStream(packet);
    QImage src = QbUtils::packetToImage(iPacket);

    if (src.isNull())
        return QbPacket();

    QImage oFrame(src.size(), src.format());

    QRgb *srcBits = (QRgb *) src.bits();
    QRgb *destBits = (QRgb *) oFrame.bits();

    if (packet.caps() != this->m_caps) {
        int cx = src.width() >> 1;
        int cy = src.height() >> 1;

        float k = 2.0 * M_PI / sqrt(cx * cx + cy * cy);

        this->m_phiTable.clear();

        for (int y = -cy; y < cy; y++)
            for (int x = -cx; x < cx; x++)
                this->m_phiTable << k * sqrt(x * x + y * y);

        this->m_caps = packet.caps();
    }

    static int tval = 0;

    float dx = 30 * sin((tval + 100) * M_PI / 128)
               + 40 * sin((tval - 10) * M_PI / 512);

    float dy = -35 * sin(tval * M_PI / 256)
               + 40 * sin((tval + 30) * M_PI / 512);

    float ripples = this->m_ripples * sin((tval - 70) * M_PI / 64);

    tval = (tval + 1) & 511;
    float *phiTable = this->m_phiTable.data();

    for (int i = 0, y = 0; y < src.height(); y++)
        for (int x = 0; x < src.width(); i++, x++) {
            float phi = ripples * phiTable[i];

            int xOrig = dx * cos(phi) + x;
            int yOrig = dy * sin(phi) + y;

            xOrig = qBound(0, xOrig, src.width());
            yOrig = qBound(0, yOrig, src.height());

            destBits[i] = srcBits[xOrig + src.width() * yOrig];
        }

    QbPacket oPacket = QbUtils::imageToPacket(oFrame, iPacket);
    qbSend(oPacket)
}
