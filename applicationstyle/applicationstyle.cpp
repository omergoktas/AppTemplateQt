#include "applicationstyle.h"

#include <QStyleFactory>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QComboBox>
#include <QMenu>
#include <QScrollBar>
#include <QMdiSubWindow>
#include <QFocusFrame>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QVariantAnimation>
#include <QLabel>
#include <QMovie>
#include <QStyleHints>

#include <private/qapplication_p.h>
#include <private/qcombobox_p.h>
#include <private/qcommonstyle_p.h>

namespace {

constexpr char buttonStyleProperty[] = "_q_ApplicationStyle_buttonStyle";
constexpr char notificationsProperty[] = "_q_ApplicationStyle_notificationsProperty";
constexpr char highlightingDisabledForCheckedStateProperty[] = "_q_ApplicationStyle_highlightingDisabledForCheckedState";
constexpr char animationProperty[] = "_q_ApplicationStyle_animationForProgressBar";

enum { NotificationsProperty = Qt::UserRole + 293373 };

void drawNotifications(QPainter* painter, const QRectF& r, const QString& text)
{
    painter->save();
    QFont f;
    f.setPixelSize(10);
    f.setWeight(QFont::DemiBold);
    QLinearGradient grad(0, 0, 0, 1);
    grad.setCoordinateMode(QGradient::ObjectMode);
    grad.setColorAt(0.0, 0xf24949);
    grad.setColorAt(1.0, 0xd94141);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(0xbf3939), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter->setBrush(grad);
    painter->drawRoundedRect(r, r.height() / 2.0, r.width() / 2.0);
    painter->setFont(f);
    painter->setPen(Qt::white);
    painter->drawText(r, text, {Qt::AlignCenter});
    painter->restore();
}

void drawArrow(Qt::ArrowType arrowType, const QStyle* style, const QStyleOption* option,
               const QRect& rect, QPainter* painter, const QWidget* widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;
    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *option;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

void drawArrow(const QStyle* style, const QStyleOptionToolButton* toolbutton,
               const QRect& rect, QPainter* painter, const QWidget* widget = 0)
{
    drawArrow(toolbutton->arrowType, style, toolbutton, rect, painter, widget);
}

bool hasVerticalParentToolBar(const QWidget* widget)
{
    if (widget == 0)
        return false;

    if (auto toolBar = qobject_cast<QToolBar*>(widget->parentWidget()))
        return toolBar->orientation() == Qt::Vertical;

    return false;
}

QPixmap pixmap(const QIcon& icon, const QSize& size, const QWidget* widget = nullptr,
               QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off)
{
    if (widget)
        return icon.pixmap(size, widget->devicePixelRatioF(), mode, state);
    return icon.pixmap(size, QGuiApplication::primaryScreen()->devicePixelRatio(), mode, state);
}

QPixmap pixmap(const QString& imagePath, const QSize& size, const QWidget* widget = nullptr,
               QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off)
{
    return pixmap(QIcon(imagePath), size, widget, mode, state);
}

QImage renderFilledImage(const QSizeF& size, const QColor& fillColor, qreal dpr)
{
    QImage dest((size * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
    dest.setDevicePixelRatio(dpr);
    dest.fill(fillColor);
    return dest;
}

QPixmap colorize(const QPixmap& px, const QColor& color)
{
    if (px.isNull())
        return px;

    QPixmap dest(px);
    QColor opaque(color);
    opaque.setAlphaF(1);

    QImage overlay = renderFilledImage(px.devicePixelRatioF() > 0 ?
                                           px.size() / px.devicePixelRatioF() : QSize(),
                                       opaque, px.devicePixelRatioF());
    {
        QPainter p(&overlay);
        p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRectF({}, px.devicePixelRatioF() > 0
                            ? px.size() / px.devicePixelRatioF()
                            : QSize()), px, px.rect());
    }

    if (color.alphaF() == 1) {
        QPixmap pix(QPixmap::fromImage(overlay));
        pix.setDevicePixelRatio(overlay.devicePixelRatio());
        return pix;
    }

    QPainter p(&dest);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(color.alphaF());
    p.drawImage(QRectF({}, px.devicePixelRatioF() > 0 ? dest.size() / px.devicePixelRatioF()
                                                      : QSize()), overlay, overlay.rect());
    p.end();

    return dest;
}

const QPalette& lightPalette()
{
    static const QPalette palette = [] ()
    {
        auto light = QGuiApplication::palette();
        light.setColor(QPalette::Active, QPalette::Text, 0x181818);
        light.setColor(QPalette::Inactive, QPalette::Text, 0x181818);
        light.setColor(QPalette::Disabled, QPalette::Text, 0x656565);
        light.setColor(QPalette::Active, QPalette::WindowText, 0x181818);
        light.setColor(QPalette::Inactive, QPalette::WindowText, 0x181818);
        light.setColor(QPalette::Disabled, QPalette::WindowText, 0x656565);
        light.setColor(QPalette::Active, QPalette::ButtonText, 0x181818);
        light.setColor(QPalette::Inactive, QPalette::ButtonText, 0x181818);
        light.setColor(QPalette::Disabled, QPalette::ButtonText, 0x656565);
        light.setColor(QPalette::Active, QPalette::PlaceholderText, 0xbfbfbf);
        light.setColor(QPalette::Inactive, QPalette::PlaceholderText, 0xbfbfbf);
        light.setColor(QPalette::Disabled, QPalette::PlaceholderText, 0xbfbfbf);
        light.setColor(QPalette::Active, QPalette::Highlight, 0x2483ec);
        light.setColor(QPalette::Inactive, QPalette::Highlight, 0x2483ec);
        light.setColor(QPalette::Disabled, QPalette::Highlight, 0x727272);
        light.setColor(QPalette::Base, 0xffffff);
        light.setColor(QPalette::AlternateBase, 0xf5f5f5);
        light.setColor(QPalette::Button, 0xf0f0f0);
        light.setColor(QPalette::Window, 0xf0f0f0);
        light.setColor(QPalette::BrightText, 0xffffff);
        light.setColor(QPalette::HighlightedText, 0xffffff);
        light.setColor(QPalette::ToolTipText, 0x181818);
        light.setColor(QPalette::ToolTipBase, 0xf0f0f0);
        light.setColor(QPalette::Link, 0x025dbf);
        light.setColor(QPalette::LinkVisited, 0xb44b46);
        return light;
    } ();
    return palette;
}

const QPalette& darkPalette()
{
    static const QPalette palette = [] ()
    {
        auto dark = QGuiApplication::palette();
        dark.setColor(QPalette::Active, QPalette::Text, 0xf0f0f0);
        dark.setColor(QPalette::Inactive, QPalette::Text, 0xf0f0f0);
        dark.setColor(QPalette::Disabled, QPalette::Text, 0x707070);
        dark.setColor(QPalette::Active, QPalette::WindowText, 0xf0f0f0);
        dark.setColor(QPalette::Inactive, QPalette::WindowText, 0xf0f0f0);
        dark.setColor(QPalette::Disabled, QPalette::WindowText, 0x707070);
        dark.setColor(QPalette::Active, QPalette::ButtonText, 0xf0f0f0);
        dark.setColor(QPalette::Inactive, QPalette::ButtonText, 0xf0f0f0);
        dark.setColor(QPalette::Disabled, QPalette::ButtonText, 0x707070);
        dark.setColor(QPalette::Active, QPalette::PlaceholderText, 0x646464);
        dark.setColor(QPalette::Inactive, QPalette::PlaceholderText, 0x646464);
        dark.setColor(QPalette::Disabled, QPalette::PlaceholderText, 0x646464);
        dark.setColor(QPalette::Active, QPalette::Highlight, 0x0078d4);
        dark.setColor(QPalette::Inactive, QPalette::Highlight, 0x0078d4);
        dark.setColor(QPalette::Disabled, QPalette::Highlight, 0x727272);
        dark.setColor(QPalette::Base, 0x282828);
        dark.setColor(QPalette::AlternateBase, 0x2f2f2f);
        dark.setColor(QPalette::Button, 0x3a3a3a);
        dark.setColor(QPalette::Window, 0x3a3a3a);
        dark.setColor(QPalette::BrightText, 0xf2f2f2);
        dark.setColor(QPalette::HighlightedText, 0xf2f2f2);
        dark.setColor(QPalette::ToolTipText, 0xf0f0f0);
        dark.setColor(QPalette::ToolTipBase, 0x3a3a3a);
        dark.setColor(QPalette::Link, 0x025dbf);
        dark.setColor(QPalette::LinkVisited, 0xb44b46);
        return dark;
    } ();
    return palette;
}
}

ApplicationStyle::ButtonStyle ApplicationStyle::buttonStyle(const QWidget* widget)
{
    const QVariant& val = widget->property(buttonStyleProperty);
    if (val.isValid())
        return val.value<ButtonStyle>();
    if (qobject_cast<const QPushButton*>(widget))
        return Push;
    if (qobject_cast<const QComboBox*>(widget))
        return Combo;
    else
        return Disclosure;
}

void ApplicationStyle::setButtonStyle(QWidget* widget, ApplicationStyle::ButtonStyle buttonStyle)
{
    widget->setProperty(buttonStyleProperty, buttonStyle);
}

bool ApplicationStyle::highlightingDisabledForCheckedState(const QWidget* widget)
{
    return widget->property(highlightingDisabledForCheckedStateProperty).value<bool>();
}

void ApplicationStyle::setHighlightingDisabledForCheckedState(QWidget* widget, bool highlightingDisabledForCheckedState)
{
    widget->setProperty(highlightingDisabledForCheckedStateProperty, highlightingDisabledForCheckedState);
}

QPointF ApplicationStyle::visualPos(Qt::LayoutDirection direction, const QRectF& boundingRect,
                                    const QPointF& logicalPos)
{
    if (direction == Qt::LeftToRight)
        return logicalPos;
    return QPointF(boundingRect.right() - logicalPos.x(), logicalPos.y());
}

ApplicationStyle::ApplicationStyle() : QProxyStyle(u"Fusion"_s)
{
}

QPalette ApplicationStyle::standardPalette() const
{
    if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        return darkPalette();
    return lightPalette();
}

QSize ApplicationStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
                                         const QSize& contentsSize, const QWidget* widget) const
{
    QSize sz = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);

    switch (type) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            sz = contentsSize;
            int maxpmw = mi->maxIconWidth;
            const QComboBox *comboBox = qobject_cast<const QComboBox*>(widget);
            int w = sz.width(), h;
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = 1; // Menu separator height
            } else {
                h = mi->fontMetrics.height() + 5;
                if (!mi->icon.isNull()) {
                    if (comboBox) {
                        const QSize& iconSize = comboBox->iconSize();
                        h = qMax(h, iconSize.height() + 6);
                        maxpmw = qMax(maxpmw, iconSize.width());
                    } else {
                        int iconExtent = proxy()->pixelMetric(PM_SmallIconSize);
                        h = qMax(h, mi->icon.actualSize(QSize(iconExtent, iconExtent)).height() + 6);
                    }
                }
            }
            if (mi->text.contains('\t'_L1))
                w += 12;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 30; // Not quite exactly as it seems to depend on other factors
            if (maxpmw)
                w += maxpmw + 6;
            // add space for a check. All items have place for a check too.
            w += 20;
            if (comboBox && comboBox->isVisible()) {
                QStyleOptionComboBox cmb;
                cmb.initFrom(comboBox);
                cmb.editable = false;
                cmb.subControls = QStyle::SC_ComboBoxEditField;
                cmb.activeSubControls = QStyle::SC_None;
                w = qMax(w, proxy()->subControlRect(QStyle::CC_ComboBox, &cmb,
                                                    QStyle::SC_ComboBoxEditField,
                                                    comboBox).width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        } break;
    case CT_MenuBarItem:
        sz = contentsSize;
        if (!sz.isEmpty())
            sz += QSize(12, 4); // Constants from QWindowsStyle
        break;
    case CT_Menu: {
        sz = contentsSize;
        QStyleHintReturnMask menuMask;
        QStyleOption myOption = *option;
        myOption.rect.setSize(sz);
        if (proxy()->styleHint(SH_Menu_Mask, &myOption, widget, &menuMask))
            sz = menuMask.region.boundingRect().size();
    } break;
    case CT_ItemViewItem:
        sz.setHeight(sz.height() + 2);
        break;
    case CT_ToolButton:
        if (hasVerticalParentToolBar(widget))
            sz = sz.transposed();
        break;
    case CT_PushButton:
        sz.setHeight(22);
        break;
    case CT_ComboBox:
        sz.setHeight(22);
        break;
    case CT_ProgressBar:
        sz.setHeight(7);
        break;
    default:
        break;
    }
    return sz;
}

QRect ApplicationStyle::subControlRect(QStyle::ComplexControl control,
                                       const QStyleOptionComplex* option,
                                       QStyle::SubControl subControl, const QWidget* widget) const
{
    QRect ret = QProxyStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *combo
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            Q_UNUSED(combo)
            if (subControl == SC_ComboBoxArrow) {
                QRectF r(option->rect);
                r.adjust(0.5, 0.5, -0.5, 0);
                ret = QRectF(r.left() + r.width() - 16.5,
                             r.top() + 0.5, 16, r.height() - 1.5).toRect();
            }
        } break;
    case CC_ToolButton:
        if (hasVerticalParentToolBar(widget))
            ret = ret.transposed();
        break;
    default:
        break;
    }

    return ret;
}

QPixmap ApplicationStyle::standardPixmap(QStyle::StandardPixmap standardPixmap,
                                         const QStyleOption* option, const QWidget* widget) const
{
    QPixmap px;

    switch (standardPixmap) {
    case SP_DockWidgetCloseButton: {
        int sz = proxy()->pixelMetric(PM_ButtonIconSize, option, widget);
        QSize size(sz, sz);
        if (auto btn = qobject_cast<const QAbstractButton*>(widget))
            size = btn->iconSize();
        px = pixmap(u":/images/close.svg"_s, QSize(sz, sz), widget);
    } break;

    case SP_LineEditClearButton: {
        int sz = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
        px = pixmap(u":/images/clear.svg"_s, QSize(sz, sz), widget);
        px = colorize(px, 0x505050);
    } break;

    case SP_MessageBoxInformation: {
        int sz = proxy()->pixelMetric(PM_MessageBoxIconSize, option, widget);
        px = pixmap(u":/images/info.svg"_s, QSize(sz, sz), widget);
    } break;

    case SP_MessageBoxWarning: {
        int sz = proxy()->pixelMetric(PM_MessageBoxIconSize, option, widget);
        px = pixmap(u":/images/warning.svg"_s, QSize(sz, sz), widget);
    } break;

    case SP_MessageBoxCritical: {
        int sz = proxy()->pixelMetric(PM_MessageBoxIconSize, option, widget);
        px = pixmap(u":/images/error.svg"_s, QSize(sz, sz), widget);
    } break;

    case SP_MessageBoxQuestion: {
        int sz = proxy()->pixelMetric(PM_MessageBoxIconSize, option, widget);
        px = pixmap(u":/images/question.svg"_s, QSize(sz, sz), widget);
    } break;

    default:
        px = QProxyStyle::standardPixmap(standardPixmap, option, widget);
        break;
    }

    if (widget)
        px.setDevicePixelRatio(widget->devicePixelRatioF());
    else
        px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}

QIcon ApplicationStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option,
                                     const QWidget* widget) const
{
    QIcon icon;
    switch (standardIcon) {
    case SP_DockWidgetCloseButton:
    case SP_MessageBoxInformation:
    case SP_MessageBoxWarning:
    case SP_MessageBoxCritical:
    case SP_MessageBoxQuestion:
        icon.addPixmap(proxy()->standardPixmap(standardIcon, option, widget));
        break;

    case SP_LineEditClearButton: {
        const QPixmap& stdPix = proxy()->standardPixmap(standardIcon, option, widget);
        icon.addPixmap(stdPix);
        icon.addPixmap(colorize(stdPix, QColor(0, 0, 0, 150)), QIcon::Active);
    } break;

    default:
        icon = QProxyStyle::standardIcon(standardIcon, option, widget);
        break;
    }
    return icon;
}

int ApplicationStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option,
                                const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_MessageBox_UseBorderForButtonSpacing:
        return false;
    case SH_MessageBox_CenterButtons:
        return false;
        // When enabled, no matter what it returns,
        // links in the message boxes do not work
        // case SH_MessageBox_TextInteractionFlags:
        //    return false;
    case SH_Widget_Animation_Duration:
        return 0; // No animation
    case SH_ToolTipLabel_Opacity:
        return 242; // About 95%
    case SH_ComboBox_AllowWheelScrolling:
        return false;
    case SH_SpinBox_AnimateButton:
        return true;
    case SH_ComboBox_ListMouseTracking:
        return false;
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_ItemView_MovementWithoutUpdatingSelection:
        return false;
    case SH_ComboBox_Popup:
        return true;
    case SH_ComboBox_PopupFrameStyle:
        return QFrame::NoFrame;
    case SH_Menu_FillScreenWithScroll:
        return false;
    case SH_FocusFrame_AboveWidget:
        return true;
    case SH_FocusFrame_Mask:
        if (widget) {
            if (QStyleHintReturnMask* mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData)) {
                mask->region = widget->rect();
                int vmargin = proxy()->pixelMetric(QStyle::PM_FocusFrameVMargin) + 1,
                        hmargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                mask->region -= QRect(widget->rect().adjusted(hmargin, vmargin, -hmargin, -vmargin));
            }
        }
        return true;
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

int ApplicationStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                                  const QWidget* widget) const
{
    switch (metric) {
    case PM_MessageBoxIconSize:
        return 40;
    case PM_SmallIconSize:
    case PM_ButtonIconSize:
    case PM_TreeViewIndentation:
    case PM_ListViewIconSize:
    case PM_TabBarIconSize:
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarExtensionExtent:
        return 22; // FIXME: Make this 18 and check if ModeSelectorPane extension icon
        // Minimum geometry for the button, not icon size,
        // icon size is determined by the toolbar's setIconSize
    case PM_ToolBarSeparatorExtent:
    case PM_DockWidgetSeparatorExtent:
    case PM_FocusFrameHMargin:
    case PM_FocusFrameVMargin:
        return 1;
    case PM_ToolBarFrameWidth: // QToolBar's contentMargins
    case PM_SplitterWidth:
        return 0;
    case PM_ToolBarItemSpacing:
        return 2;
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarHandleExtent:
        return 9;
    case PM_ToolTipLabelFrameWidth:
        return 1;
    case PM_MenuVMargin:
        return 4;
    case PM_MenuScrollerHeight:
        return 15;
    case PM_MenuButtonIndicator:
        return 12;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void ApplicationStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_Frame: {
        if (widget && widget->inherits("QComboBoxPrivateContainer")){
            QStyleOption copy = *option;
            copy.state |= State_Raised;
            proxy()->drawPrimitive(PE_PanelMenu, &copy, painter, widget);
            break;
        }
        painter->save();
        painter->setPen(QPen(QColor(0xb6b6b6), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter->drawRect(QRectF(option->rect).adjusted(0, 0, -0.5, -0.5));
        painter->restore();
    } break;
    case PE_PanelLineEdit: {
        if (auto lineEdit = qobject_cast<const QLineEdit*>(widget)) {
            if (lineEdit->hasFrame())
                QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
    } break;
    case PE_PanelTipLabel:
        painter->fillRect(option->rect, option->palette.brush(QPalette::ToolTipBase));
        break;
    case PE_PanelMenu: {
        painter->save();
        painter->setPen(0xb0b0b0);
        painter->setBrush(option->palette.window());
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
        painter->restore();
    } break;
    case PE_IndicatorMenuCheckMark: {
        painter->save();
        QColor pc;
        if (option->state & State_On)
            pc = option->palette.highlightedText().color();
        else
            pc = option->palette.text().color();

        painter->setPen(pc);
        painter->setFont(QFont());
        painter->drawText(option->rect.adjusted(-2, 1, -2, 1), u"✓"_s, QTextOption(Qt::AlignCenter));
        painter->restore();
    } break;
    case PE_IndicatorArrowDown: {
        if (option->rect.width() <= 1 || option->rect.height() <= 1)
            break;
        if (!(option->state & State_Enabled))
            break;

        painter->save();

        enum { ARROW_LENGTH = 3 };
        QRectF r(0, 0, ARROW_LENGTH, ARROW_LENGTH / 2.0);
        r.moveCenter(QRectF(option->rect).center());

        const QColor& textColor = (option->state & State_Sunken)
                ? option->palette.buttonText().color().darker()
                : option->palette.buttonText().color();
        QPen arrowPen(textColor);
        arrowPen.setWidthF(1.3);
        arrowPen.setCapStyle(Qt::RoundCap);
        arrowPen.setJoinStyle(Qt::MiterJoin);

        QPointF points[] = {{0, 0}, {ARROW_LENGTH / 2.0, ARROW_LENGTH / 2.0}, {ARROW_LENGTH, 0}};
        points[0] += r.topLeft(); points[1] += r.topLeft(); points[2] += r.topLeft();
        painter->setPen(arrowPen);
        painter->setBrush(textColor);
        painter->drawPolygon(points, 3);
        painter->restore();
    } break;
    case PE_IndicatorToolBarSeparator: {
        QPainterPath path;
        QRectF r(option->rect);
        if (option->state & State_Horizontal) {
            qreal xpoint = r.center().x();
            path.moveTo(xpoint + 0.5, r.top() + 4);
            path.lineTo(xpoint + 0.5, r.bottom() - 4);
        } else {
            qreal ypoint = r.center().y();
            path.moveTo(r.left() + 4 , ypoint + 0.5);
            path.lineTo(r.right() - 4, ypoint + 0.5);
        }
        QPainterPathStroker theStroker;
        theStroker.setCapStyle(Qt::FlatCap);
        // theStroker.setDashPattern(QVector<qreal>() << 1 << 2);
        path = theStroker.createStroke(path);
        painter->fillPath(path, QColor(0x00, 0x00, 0x00, 0x50));
    } break;
    case PE_PanelButtonTool:
        if (widget && widget->objectName() == "qt_toolbar_ext_button"_L1)
            break;
        if (widget && widget->parent() && widget->parent()->objectName() == "modeSelectorPane"_L1) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::NoBrush);
            if (option->state & State_Sunken)
                painter->setBrush(QColor(0x00, 0x00, 0x00, 0x80));
            else if (option->state & State_On)
                painter->setBrush(QColor(0x00, 0x00, 0x00, 0x60));
            else if (option->state & State_MouseOver)
                painter->setBrush(QColor(0x00, 0x00, 0x00, 0x20));
            painter->drawRoundedRect(QRectF(option->rect), 7, 7);
            painter->restore();
            break;
        }
        if (hasVerticalParentToolBar(widget)) {
            painter->save();
            painter->translate(0, widget->height());
            painter->rotate(-90);
            QProxyStyle::drawPrimitive(element, option, painter, widget);
            painter->restore();
        } break;
    case PE_PanelButtonCommand: {
        if (auto combo = qobject_cast<const QComboBox*>(widget)) {
            if (!combo->hasFrame())
                break;
        }
        QRectF rect(option->rect);
        ButtonStyle style = buttonStyle(widget);
        bool hasMenu = false;
        bool isFlat = false;
        bool isDefault = false;
        bool hasHover = option->state & QStyle::State_MouseOver;
        bool isEnabled = option->state & QStyle::State_Enabled;
        bool isDown = (option->state & QStyle::State_Sunken)
                || (!highlightingDisabledForCheckedState(widget) && option->state & QStyle::State_On);
        bool hasFocus = (option->state & QStyle::State_HasFocus)
                && (option->state & QStyle::State_KeyboardFocusChange);

        if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            hasMenu = button->features & QStyleOptionButton::HasMenu;
            isFlat = button->features & QStyleOptionButton::Flat;
            isDefault = button->features & QStyleOptionButton::DefaultButton;
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        if (!isFlat /*|| isDown*/) {
            switch (style) {
            case Push: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.1, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.9, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(1.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x15) : QColor(0x00, 0x00, 0x00, 0x08));
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
                painter->setBrush(QColor(isEnabled ? QColor(0x00, 0x00, 0x00, 0x09) : QColor(0x00, 0x00, 0x00, 0x05)));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? isDown ? 0x3280f7 : ((isDefault || hasFocus) ? 0x5094f7 : 0xd8d8d8) : 0xebebeb);
                borderGrad.setColorAt(0.1, isEnabled ? isDown ? 0x2e7bf3 : ((isDefault || hasFocus) ? 0x468ef8 : 0xd0d0d0) : 0xe7e7e7);
                borderGrad.setColorAt(0.9, isEnabled ? isDown ? 0x1a5fda : ((isDefault || hasFocus) ? 0x196dfb : 0xd0d0d0) : 0xe7e7e7);
                borderGrad.setColorAt(1.0, isEnabled ? isDown ? 0x1659d5 : ((isDefault || hasFocus) ? 0x1367fb : 0xbcbcbc) : 0xdddddd);
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

                // Draw body
                QLinearGradient bodyGrad(0, 0, 0, 1);
                bodyGrad.setCoordinateMode(QGradient::ObjectMode);
                bodyGrad.setColorAt(0.0, isEnabled ? isDown ? 0x5496f9 : ((isDefault || hasFocus) ? 0x6fa7f8 : 0xffffff) : 0xffffff);
                bodyGrad.setColorAt(0.9, isEnabled ? isDown ? 0x1c65dd : ((isDefault || hasFocus) ? 0x176ffb : 0xffffff) : 0xffffff);
                bodyGrad.setColorAt(1.0, isEnabled ? isDown ? 0x1c65dd : ((isDefault || hasFocus) ? 0x176ffb : 0xffffff) : 0xffffff);
                painter->setBrush(bodyGrad);
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);
            } break;

            case Disclosure: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.1, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.9, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(1.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x15) : QColor(0x00, 0x00, 0x00, 0x08));
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
                painter->setBrush(QColor(isEnabled ? QColor(0x00, 0x00, 0x00, 0x09) : QColor(0x00, 0x00, 0x00, 0x05)));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? 0xd8d8d8 : 0xebebeb);
                borderGrad.setColorAt(0.1, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(0.9, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(1.0, isEnabled ? 0xbcbcbc : 0xdddddd);
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

                // Draw body
                painter->setBrush(QColor(isEnabled ? isDown ? 0xf0f0f0 : 0xffffff : 0xffffff));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);
            } break;

            case Help: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                qreal radius = qMin(rect.width() - 1, rect.height() - 1.5) / 2.0;
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.1, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.9, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(1.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x15) : QColor(0x00, 0x00, 0x00, 0x08));
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), radius + 0.5, radius + 0.5);
                painter->setBrush(QColor(isEnabled ? QColor(0x00, 0x00, 0x00, 0x09) : QColor(0x00, 0x00, 0x00, 0x05)));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), radius + 0.5, radius + 0.5);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? 0xd8d8d8 : 0xebebeb);
                borderGrad.setColorAt(0.1, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(0.9, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(1.0, isEnabled ? 0xbcbcbc : 0xdddddd);
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), radius, radius);

                // Draw body
                painter->setBrush(QColor(isEnabled ? isDown ? 0xf0f0f0 : 0xffffff : 0xffffff));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5),  radius - 0.5,  radius - 0.5);
            } break;

            case TexturedRounded: {
                // Draw drop shadow
                rect.adjust(0.5, 0, -0.5, 0);
                QLinearGradient shadowGrad(0, 0, 1, 0);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0, QColor(0x20, 0x20, 0x20, 0x12));
                shadowGrad.setColorAt(0.05, QColor(0x20, 0x20, 0x20, 0x10));
                shadowGrad.setColorAt(0.5, QColor(0x20, 0x20, 0x20, 0x10));
                shadowGrad.setColorAt(0.95, QColor(0x20, 0x20, 0x20, 0x10));
                shadowGrad.setColorAt(1, QColor(0x20, 0x20, 0x20, 0x12));
                QPainterPath dropShadowPath;
                dropShadowPath.addRect(3.5, option->rect.bottom() + 0.5, option->rect.width() - 7, 1);
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawPath(dropShadowPath);

                // Draw shadow
                QLinearGradient darkGrad(0, 0, 0, 1);
                darkGrad.setCoordinateMode(QGradient::ObjectMode);
                darkGrad.setColorAt(0.85, QColor(0x30, 0x30, 0x30, 0x20));
                darkGrad.setColorAt(1, QColor(0x00, 0x00, 0x00, 0x3f));
                QPainterPath shadowPath;
                shadowPath.addRoundedRect(rect.adjusted(0, 0.5, 0, -0.5), 4, 4);
                painter->setPen(Qt::NoPen);
                painter->setBrush(darkGrad);
                painter->drawPath(shadowPath);

                // Draw body
                QLinearGradient midGrad(0, 0, 0, 1);
                midGrad.setCoordinateMode(QGradient::ObjectMode);
                midGrad.setColorAt(0, 0xe4e4e4);
                midGrad.setColorAt(1, 0xdedede);
                QLinearGradient buttonGrad(0, 0, 0, 1);
                buttonGrad.setCoordinateMode(QGradient::ObjectMode);
                buttonGrad.setColorAt(0, 0xfdfdfd);
                buttonGrad.setColorAt(1, 0xf3f3f3);
                QPainterPath bodyPath;
                bodyPath.addRoundedRect(rect.adjusted(0.5, 1, -0.5, -1), 3.5, 3.5);
                painter->setPen(Qt::NoPen);
                painter->setBrush(isDown ? midGrad : buttonGrad);
                painter->drawPath(bodyPath);
                // NOTE: QRect's bottom() and bottomLeft()... are different from QRectF, for historical reasons
                // as stated in the docs, those functions return top() + height() - 1 (QRect)

                // Draw glowing for pressed state
                if (isDown) {
                    QPainterPath glowPathUp;
                    glowPathUp.addRoundedRect(rect.adjusted(0, 1.5, 0, 1), 4.5, 4.5);
                    QPainterPath glowPathDown;
                    glowPathDown.addRoundedRect(rect.adjusted(0, 2, 0, 1), 4.5, 4.5);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QColor(0xebebeb));
                    painter->drawPath(bodyPath.subtracted(glowPathDown));
                    painter->setBrush(QColor(0xf3f3f3));
                    painter->drawPath(bodyPath.subtracted(glowPathUp));
                }
            } break;

            case Combo: {
                rect.adjust(0.5, 0.5, -0.5, 0);
                // Draw shadows
                QLinearGradient shadowGrad(0, 0, 0, 1);
                shadowGrad.setCoordinateMode(QGradient::ObjectMode);
                shadowGrad.setColorAt(0.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.1, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(0.9, isEnabled ? QColor(0x00, 0x00, 0x00, 0x07) : QColor(0x00, 0x00, 0x00, 0x05));
                shadowGrad.setColorAt(1.0, isEnabled ? QColor(0x00, 0x00, 0x00, 0x15) : QColor(0x00, 0x00, 0x00, 0x08));
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowGrad);
                painter->drawRoundedRect(rect.adjusted(0, 0, 0, -0.5), 4, 4);
                painter->setBrush(QColor(isEnabled ? QColor(0x00, 0x00, 0x00, 0x09) : QColor(0x00, 0x00, 0x00, 0x05)));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, 0), 4, 4);

                // Draw border
                QLinearGradient borderGrad(0, 0, 0, 1);
                borderGrad.setCoordinateMode(QGradient::ObjectMode);
                borderGrad.setColorAt(0.0, isEnabled ? 0xd8d8d8 : 0xebebeb);
                borderGrad.setColorAt(0.1, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(0.9, isEnabled ? 0xd0d0d0 : 0xe7e7e7);
                borderGrad.setColorAt(1.0, isEnabled ? 0xbcbcbc : 0xdddddd);
                painter->setBrush(borderGrad);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -1), 3.5, 3.5);

                // Draw body
                painter->setBrush(QColor(isEnabled ? isDown ? 0xf0f0f0 : 0xffffff : 0xffffff));
                painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1.5), 3, 3);

                // Draw handle
                if (isEnabled) {
                    QRectF hr(rect.left() + rect.width() - 16.5, rect.top() + 0.5, 16, rect.height() - 1.5);
                    painter->setClipRect(hr);
                    // Draw border
                    QPainterPath hp;
                    hp.moveTo(hr.topLeft() - QPointF(1, 0));
                    hp.lineTo(hr.topRight() - QPointF(7, 0));
                    hp.cubicTo(hr.topRight(), hr.topRight(),
                               hr.topRight() + QPointF(0, 7));
                    hp.lineTo(hr.bottomRight() - QPointF(0, 7));
                    hp.cubicTo(hr.bottomRight(), hr.bottomRight(),
                               hr.bottomRight() - QPointF(7, 0));
                    hp.lineTo(hr.bottomLeft() - QPointF(1, 0));
                    hp.lineTo(hr.topLeft() - QPointF(1, 0));
                    QLinearGradient borderGrad2(0, 0, 0, 1);
                    borderGrad2.setCoordinateMode(QGradient::ObjectMode);
                    borderGrad2.setColorAt(0.0, isDown ? 0x3280f7 : 0x5496f4);
                    borderGrad2.setColorAt(0.1, isDown ? 0x2e7bf3 : 0x4a91f5);
                    borderGrad2.setColorAt(0.9, isDown ? 0x195cd3 : 0x2271f7);
                    borderGrad2.setColorAt(1.0, isDown ? 0x1450bf : 0x1d6cf7);
                    painter->setBrush(borderGrad2);
                    painter->drawPath(hp);

                    // Draw body
                    hp.clear();
                    hr.adjust(0, 0.5, -0.5, -0.5);
                    hp.moveTo(hr.topLeft() - QPointF(1, 0));
                    hp.lineTo(hr.topRight() - QPointF(6, 0));
                    hp.cubicTo(hr.topRight(), hr.topRight(),
                               hr.topRight() + QPointF(0, 6));
                    hp.lineTo(hr.bottomRight() - QPointF(0, 6));
                    hp.cubicTo(hr.bottomRight(), hr.bottomRight(),
                               hr.bottomRight() - QPointF(6, 0));
                    hp.lineTo(hr.bottomLeft() - QPointF(1, 0));
                    hp.lineTo(hr.topLeft() - QPointF(1, 0));
                    QLinearGradient bodyGrad(0, 0, 0, 1);
                    bodyGrad.setCoordinateMode(QGradient::ObjectMode);
                    bodyGrad.setColorAt(0.0, isDown ? 0x5496f9 : 0x71a9f5);
                    bodyGrad.setColorAt(0.9, isDown ? 0x1a5ece : 0x2173f7);
                    bodyGrad.setColorAt(1.0, isDown ? 0x1a5ece : 0x2173f7);
                    painter->setBrush(bodyGrad);
                    painter->drawPath(hp);
                    painter->setClipping(false);
                }
            } break;

            default:
                break;
            }
        }

        if (isEnabled && hasMenu && (hasHover || isDown)) {
            if (auto button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
                int mbi = proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator, option);
                QStyleOptionButton newBtn = *button;
                newBtn.rect = QRect(rect.right() - mbi, rect.bottom() - mbi, mbi, mbi);
                proxy()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, painter);
            }
        }

        painter->restore();
    } break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option,
                                   QPainter* painter, const QWidget* widget) const
{
    const QRectF r(option->rect);

    switch (element) {
    case CE_MenuItem:
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuTearoff:
    case CE_MenuScroller:
        if (const QStyleOptionMenuItem* m
                = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            painter->save();
            const int macItemFrame = 1; // menu item frame width
            const int macItemHMargin = 2; // menu item hor text margin
            const int macRightBorder = 12; // right border on mac
            QStyleOptionMenuItem mi(*m);
            const bool active = mi.state & State_Selected;
            if (active)
                painter->fillRect(mi.rect, mi.palette.highlight());
            if (element == CE_MenuTearoff) {
                painter->setPen(QPen(mi.palette.dark().color(), 1, Qt::DashLine));
                painter->drawLine(mi.rect.x() + 2, mi.rect.y() + mi.rect.height() / 2 - 1,
                                  mi.rect.x() + mi.rect.width() - 4,
                                  mi.rect.y() + mi.rect.height() / 2 - 1);
                painter->setPen(QPen(mi.palette.light().color(), 1, Qt::DashLine));
                painter->drawLine(mi.rect.x() + 2, mi.rect.y() + mi.rect.height() / 2,
                                  mi.rect.x() + mi.rect.width() - 4,
                                  mi.rect.y() + mi.rect.height() / 2);
            } else if (element == CE_MenuScroller) {
                const QSize scrollerSize = QSize(10, 8);
                const int scrollerVOffset = 5;
                const int left = mi.rect.x() + (mi.rect.width() - scrollerSize.width()) / 2;
                const int right = left + scrollerSize.width();
                int top;
                int bottom;
                if (option->state & State_DownArrow) {
                    bottom = mi.rect.y() + scrollerVOffset;
                    top = bottom + scrollerSize.height();
                } else {
                    bottom = mi.rect.bottom() - scrollerVOffset;
                    top = bottom - scrollerSize.height();
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.moveTo(left, bottom);
                path.lineTo(right, bottom);
                path.lineTo((left + right) / 2, top);
                painter->fillPath(path, option->palette.buttonText());
                painter->restore();
            } else if (element != CE_MenuItem) {
                painter->restore();
                break;
            }
            if (mi.menuItemType == QStyleOptionMenuItem::Separator) {
                const QRect separatorRect = QRect(mi.rect.left(), mi.rect.center().y(), mi.rect.width(), 2);
                painter->fillRect(separatorRect, 0xd5d5d5);
                painter->restore();
                break;
            }
            const int maxpmw = 20/*mi.maxIconWidth*/ ;
            const bool enabled = mi.state & State_Enabled;
            int xpos = mi.rect.x() + 18;
            int checkcol = maxpmw;
            if (!enabled)
                painter->setPen(mi.palette.text().color());
            else if (active)
                painter->setPen(mi.palette.highlightedText().color());
            else
                painter->setPen(mi.palette.buttonText().color());
            if (mi.checked) {
                QStyleOption checkmarkOpt;
                checkmarkOpt.initFrom(widget);
                const int mw = checkcol + macItemFrame;
                const int mh = mi.rect.height() + macItemFrame;
                const int xp = mi.rect.x() + macItemFrame;
                checkmarkOpt.rect = QRect(xp, mi.rect.y() - checkmarkOpt.fontMetrics.descent(), mw, mh);
                checkmarkOpt.state.setFlag(State_On, active);
                checkmarkOpt.state.setFlag(State_Enabled, enabled);
                checkmarkOpt.state |= State_Small;
                // We let drawPrimitive(PE_IndicatorMenuCheckMark) pick the right color
                checkmarkOpt.palette.setColor(QPalette::HighlightedText, painter->pen().color());
                checkmarkOpt.palette.setColor(QPalette::Text, painter->pen().color());
                proxy()->drawPrimitive(PE_IndicatorMenuCheckMark, &checkmarkOpt, painter, widget);
            }
            bool dis = !(mi.state & State_Enabled);
            bool act = mi.state & State_Selected;
            if (!mi.icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap px;

                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                QSize iconSize(smallIconSize, smallIconSize);

                if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget))
                    iconSize = combo->iconSize();

                px = pixmap(mi.icon, iconSize, widget,
                            mode, mi.checked ? QIcon::On : QIcon::Off);
                int pixw = px.devicePixelRatioF() > 0 ? px.width() / px.devicePixelRatioF() : 1;
                int pixh = px.devicePixelRatioF() > 0 ? px.height() / px.devicePixelRatioF() : 1;
                QRect cr(xpos, mi.rect.y(), checkcol, mi.rect.height());
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                painter->drawPixmap(pmr.topLeft(), px);
                xpos += pixw + 6;
            }

            QString s = mi.text;
            const auto text_flags = Qt::AlignVCenter | Qt::TextHideMnemonic
                    | Qt::TextSingleLine | Qt::AlignAbsolute;
            int yPos = mi.rect.y();
            const bool isSubMenu = mi.menuItemType == QStyleOptionMenuItem::SubMenu;
            const int tabwidth = isSubMenu ? 9 : mi.reservedShortcutWidth;
            QString rightMarginText;
            if (isSubMenu)
                rightMarginText = u"▶"_s; // U+25B6: BLACK RIGHT-POINTING TRIANGLE
            // If present, save and remove embedded shorcut from text
            const qsizetype tabIndex = s.indexOf('\t'_L1);
            if (tabIndex >= 0) {
                if (!isSubMenu) // ... but ignore it if it's a submenu.
                    rightMarginText = s.mid(tabIndex + 1);
                s = s.left(tabIndex);
            }
            if (!rightMarginText.isEmpty()) {
                painter->setFont(qt_app_fonts_hash()->value("QMenuItem"_ba, painter->font()));
                int xp = mi.rect.right() - tabwidth - macRightBorder + 2;
                if (!isSubMenu)
                    xp -= macItemHMargin + macItemFrame + 3; // Adjust for shortcut
                painter->drawText(xp, yPos, tabwidth, mi.rect.height(), text_flags | Qt::AlignRight, rightMarginText);
            }
            if (!s.isEmpty()) {
                const int xm = macItemFrame + maxpmw + macItemHMargin;
                QFont myFont = mi.font;
                myFont.setPixelSize(QFontInfo(mi.font).pixelSize());
                painter->setFont(myFont);
                painter->drawText(xpos, yPos, mi.rect.width() - xm - tabwidth + 1,
                                  mi.rect.height(), text_flags, s);
            }
            painter->restore();
        } break;
    case CE_ComboBoxLabel:
        if (const auto *cb
                = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            auto copy = *cb;
            copy.direction = Qt::LeftToRight;
            // The rectangle will be adjusted to SC_ComboBoxEditField with comboboxEditBounds()
            QProxyStyle::drawControl(element, &copy, painter, widget);
        } break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QRectF ir = button->rect;
            uint tf = Qt::AlignVCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextShowMnemonic;
            else
                tf |= Qt::TextHideMnemonic;
            if (!button->icon.isNull()) {
                //Center both icon and text
                QPointF point;
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (button->state & State_On)
                    state = QIcon::On;

                QPixmap px = pixmap(button->icon, button->iconSize, widget, mode, state);
                if (button->state & State_Sunken)
                    px = colorize(px, QColor(0x00, 0x00, 0x00, 0x30));
                qreal w = px.devicePixelRatioF() > 0 ? px.width() / px.devicePixelRatioF() : 1;
                qreal h = px.devicePixelRatioF() > 0 ? px.height() / px.devicePixelRatioF() : 1;
                if (!button->text.isEmpty())
                    w += button->fontMetrics.boundingRect(option->rect, tf, button->text).width() + 3;
                point = QPointF(ir.x() + ir.width() / 2 - w / 2,
                                ir.y() + ir.height() / 2 - h / 2);
                w = px.devicePixelRatioF() > 0 ? px.width() / px.devicePixelRatioF() : 1;
                if (button->direction == Qt::RightToLeft)
                    point.rx() += w;
                painter->drawPixmap(visualPos(button->direction, button->rect, point), px, px.rect());
                if (button->direction == Qt::RightToLeft)
                    ir.translate(ir.x() - point.x() - 2, 0);
                else
                    ir.translate(point.x() - ir.x() + w + 3, 0);
                // left-align text if there is
                if (!button->text.isEmpty())
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            if (button->features & QStyleOptionButton::HasMenu)
                ir = ir.adjusted(0, 0, -proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget), 0);

            // Draw item text
            QStyleOptionButton copy(*button);
            if (buttonStyle(widget) == Push) {
                bool isFlat = button->features & QStyleOptionButton::Flat;
                bool isDefault = button->features & QStyleOptionButton::DefaultButton;
                bool isEnabled = button->state & QStyle::State_Enabled;
                bool isDown = (button->state & QStyle::State_Sunken)
                        || (!highlightingDisabledForCheckedState(widget) && button->state & QStyle::State_On);
                bool hasFocus = (button->state & QStyle::State_HasFocus)
                        && (button->state & QStyle::State_KeyboardFocusChange);

                if (isEnabled && (!isFlat || isDown)) {
                    if (isDown)
                        copy.palette.setColor(QPalette::ButtonText, 0xe4effe);
                    else if (isDefault || hasFocus)
                        copy.palette.setColor(QPalette::ButtonText, Qt::white);
                }
            }
            ApplicationStyle::drawItemText(painter, ir, tf, copy.palette, copy.state & State_Enabled,
                                           copy.text, QPalette::ButtonText);

            if (widget) {
                const QVariant& value = widget->property(notificationsProperty);
                if (value.isValid()) {
                    QRectF x = option->rect;
                    x = QRectF(QPointF(x.right() - 14, x.center().y() - 2 - option->fontMetrics.height() / 2.0),
                               QSizeF(12, 12));
                    drawNotifications(painter, x, value.toString());
                }
            }
        } break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton* opt
                = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            painter->save();
            QStyleOptionToolButton copy(*opt);
            QStyleOptionToolButton* toolbutton = &copy;
            if (hasVerticalParentToolBar(widget)) {
                painter->translate(0, widget->height());
                painter->rotate(-90);
            }
            if (widget && widget->parent() && widget->parent()->objectName() == "modeSelectorPane"_L1) {
                toolbutton->rect.adjust(4, 0, 0, 0);
                if (toolbutton->state & State_On || toolbutton->state & State_Sunken)
                    toolbutton->palette.setColor(QPalette::ButtonText, Qt::white);
            }
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                painter->setFont(toolbutton->font);
                proxy()->drawItemText(painter, rect, alignment, toolbutton->palette,
                                      option->state & State_Enabled, toolbutton->text,
                                      QPalette::ButtonText);
            } else {
                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;
                if (!toolbutton->icon.isNull()) {
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (widget && widget->parent() && widget->parent()->objectName()
                            == "modeSelectorPane"_L1 && toolbutton->state & State_Sunken) {
                        state = QIcon::On;
                    }
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((option->state & State_MouseOver) && (option->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = pixmap(toolbutton->icon,
                                toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                widget, mode, state);
                    pmSize = pm.devicePixelRatioF() > 0 ? pm.size() / pm.devicePixelRatioF() : QSize();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    painter->setFont(toolbutton->font);
                    QRect pr = rect,
                            tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(0, pr.height() - 1, 0, -1);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(pr.width(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, QStyle::visualRect(option->direction, rect,
                                                                                pr), Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    const QString text = reinterpret_cast<const QCommonStylePrivate*>
                            (QStylePrivate::get(this))->toolButtonElideText(toolbutton, tr, alignment);
                    proxy()->drawItemText(painter, QStyle::visualRect(option->direction, rect, tr),
                                          alignment, toolbutton->palette,
                                          toolbutton->state & State_Enabled, text,
                                          QPalette::ButtonText);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(proxy(), toolbutton, rect, painter, widget);
                    } else {
                        proxy()->drawItemPixmap(painter, rect, Qt::AlignCenter, pm);
                    }
                }
            }
            painter->restore();
        } break;
    case CE_ToolBar:
        if (const auto *cb = qstyleoption_cast<const QStyleOptionToolBar*>(option)) {
            painter->save();
            const QPointF finalPoint = (cb->state & State_Horizontal) ? QPointF(0, 1) : QPointF(1, 0);
            QLinearGradient gradient(QPointF(), finalPoint);
            gradient.setCoordinateMode(QGradient::ObjectMode);
            gradient.setColorAt(0, 0xffffff);
            gradient.setColorAt(1, 0xe3e3e3);
            painter->setBrush(gradient);
            painter->setPen(QPen(QColor(0xb6b6b6), 1, Qt::SolidLine, Qt::FlatCap,
                                 Qt::MiterJoin));
            painter->drawRect(cb->rect);
            painter->restore();
        } break;
    case CE_RubberBand:
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            QPen pen(QColor(0, 0, 0, 200), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->setBrush(QColor(0, 0, 0, 30));
            painter->drawRect(option->rect);
            painter->restore();
        } break;
    case CE_Splitter: {
        if (option->state & State_Horizontal) {
            for (int i = -6; i < 12; i += 3)
                painter->fillRect(r.center().x(), r.center().y() + i, 1, 1, QColor(0, 0, 0, 150));
        } else {
            for (int i = -6; i < 12; i += 3)
                painter->fillRect(r.center().x() + i, r.center().y(), 1, 1, QColor(0, 0, 0, 150));
        }
    } break;
    case CE_FocusFrame:
        painter->fillRect(r.adjusted(1, 1, -1, -1), option->palette.highlight());
        break;
    case CE_ProgressBarLabel:
    case CE_ProgressBarGroove:
        // Do nothing. All done in CE_ProgressBarContents. Only keep these for proxy style overrides.
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar* opt = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
            static const QColor backgroundColor = QColor(0x00, 0x00, 0x00, 0x12);
            static const QColor borderColor = QColor(0x00, 0x00, 0x00, 0x40);
            static const QColor indeterminateColor = QColor(0xff, 0xff, 0xff, 0x45);
            const QColor progressColor = opt->invertedAppearance ? opt->palette.button().color()
                                                                 : QColor(0x419BF9);
            const bool isIndeterminate = (opt->minimum == 0 && opt->maximum == 0);
            const qreal borderRadius = opt->rect.height() / 2.0;
            auto animation = opt->styleObject
                    ? opt->styleObject->property(animationProperty).value<QVariantAnimation*>() : nullptr;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            // Limit drawing area
            QPainterPath path;
            path.addRoundedRect(opt->rect, borderRadius, borderRadius);
            painter->setClipPath(path);

            // Draw the progress
            if (isIndeterminate) {
                if (animation == 0 && qobject_cast<QWidget*>(opt->styleObject)) {
                    animation = new QVariantAnimation(opt->styleObject);
                    animation->setStartValue(0.0);
                    animation->setEndValue(1.0);
                    animation->setDuration(1400);
                    animation->setLoopCount(-1);
                    connect(animation, &QVariantAnimation::valueChanged,
                            qobject_cast<QWidget*>(opt->styleObject), qOverload<>(&QWidget::update));
                    opt->styleObject->setProperty(animationProperty, QVariant::fromValue(animation));
                }
                do {
                    if (animation == 0)
                        break;
                    animation->start(); // Safe to call, since it does nothing if already running
                    const qreal mover = opt->rect.width() * animation->currentValue().toReal();
                    const qreal glareLength = opt->rect.width() * 0.66;
                    const QRectF glareRect(opt->rect.left() - glareLength + mover, opt->rect.top(),
                                           glareLength, opt->rect.height());

                    QLinearGradient glareGrad(0.0, 0.5, 1.0, 0.5);
                    glareGrad.setCoordinateMode(QGradient::ObjectMode);
                    glareGrad.setColorAt(0, Qt::transparent);
                    glareGrad.setColorAt(0.4, indeterminateColor);
                    glareGrad.setColorAt(0.5, indeterminateColor);
                    glareGrad.setColorAt(0.6, indeterminateColor);
                    glareGrad.setColorAt(1, Qt::transparent);

                    painter->fillRect(opt->rect, progressColor);
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(glareGrad);
                    painter->drawRect(glareRect);
                    painter->drawRect(glareRect.adjusted(opt->rect.width(), 0, opt->rect.width(), 0));
                } while(false);
            } else {
                if (animation)
                    animation->stop();
                if (const qreal length = opt->maximum - opt->minimum) {
                    const qreal progressWidth = opt->rect.width() * opt->progress / length;
                    painter->fillRect(opt->rect, backgroundColor);
                    painter->fillRect(opt->rect.adjusted(0, 0, -opt->rect.width() + progressWidth, 0),
                                      progressColor);
                }
            }

            // Draw the frame
            painter->setPen(borderColor);
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect((QRectF)opt->rect, borderRadius, borderRadius);
            painter->restore();
        }
        break;
    case CE_TabBarTabLabel:
        painter->save();
        QProxyStyle::drawControl(element, option, painter, widget);
        painter->restore();
        if (auto bar = qobject_cast<const QTabBar*>(widget)) {
            if (auto opt = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                const QVariant& value = bar->tabData(bar->tabAt(opt->rect.center()));
                if (value.isValid()) {
                    QRectF x = proxy()->subElementRect(SE_TabBarTabText, option, widget);
                    x = QRectF(QPointF(x.right() - 3, x.center().y() - 3 - option->fontMetrics.height() / 2.0),
                               QSizeF(12, 12));
                    drawNotifications(painter, x, value.toString());
                }
            }
        } break;
    case CE_ItemViewItem:
        painter->save();
        QProxyStyle::drawControl(element, option, painter, widget);
        painter->restore();
        if (widget) {
            if (auto opt = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
                const QVariant& value = opt->index.data(NotificationsProperty);
                if (value.isValid()) {
                    QRectF x = proxy()->subElementRect(SE_ItemViewItemFocusRect, option, widget);
                    x = QRectF(QPointF(x.right() - 15, x.center().y() - 6), QSizeF(12, 12));
                    drawNotifications(painter, x, value.toString());
                }
            }
        } break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawComplexControl(QStyle::ComplexControl control,
                                          const QStyleOptionComplex* option,
                                          QPainter* painter, const QWidget* widget) const
{
    switch (control) {
    case CC_ComboBox:
        if (auto combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            QStyleOptionComboBox copy = *combo;
            copy.subControls &= ~SC_ComboBoxArrow;
            copy.editable = false;
            QProxyStyle::drawComplexControl(control, &copy, painter, widget);
            if (combo->subControls & SC_ComboBoxArrow) {
                QColor arrowColor = option->palette.buttonText().color();
                arrowColor.setAlpha(230);
                if (buttonStyle(widget) == Combo && combo->frame)
                    arrowColor = Qt::white;
                QRectF dr = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxArrow, widget);
                QPainterPath pathDown;
                pathDown.moveTo(0.5, 0.5);
                pathDown.lineTo(3.5, 3.5);
                pathDown.lineTo(6.5, 0.5);
                QPainterPath pathUp;
                pathUp.moveTo(0.5, 3.5);
                pathUp.lineTo(3.5, 0.5);
                pathUp.lineTo(6.5, 3.5);
                painter->setPen(QPen(arrowColor, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
                QPointF tl(dr.topLeft() + QPointF(dr.width() / 2.0 - 3.5, dr.height() / 2.0 - 2.25));
                painter->translate(tl - QPointF(0, 3.5));
                painter->drawPath(pathUp);
                painter->translate(QPointF(0, 7.5));
                painter->drawPath(pathDown);
            }
            painter->restore();
        } break;
    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

void ApplicationStyle::drawItemText(QPainter* painter, const QRect& rect, int alignment,
                                    const QPalette& pal, bool enabled, const QString& text,
                                    QPalette::ColorRole textRole) const
{
    ApplicationStyle::drawItemText(painter, QRectF(rect), alignment, pal, enabled, text, textRole);
}

void ApplicationStyle::drawItemText(QPainter* painter, const QRectF& rect, int alignment,
                                    const QPalette& pal, bool enabled, const QString& text,
                                    QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    QPen savedPen = painter->pen();
    if (textRole != QPalette::NoRole) {
        painter->setPen(QPen(pal.brush(enabled ? QPalette::Active : QPalette::Disabled, textRole),
                             savedPen.widthF()));
    }
    painter->drawText(rect, alignment, text);
    painter->setPen(savedPen);
}

void ApplicationStyle::drawItemPixmap(QPainter* painter, const QRect& rect, int alignment,
                                      const QPixmap& px) const
{
    // Workaround for bad QMovie painting
    QPixmap copy(px);
    if (auto label = dynamic_cast<const QLabel*>(painter->device())) {
        if (label->movie() && label->movie()->isValid() && label->movie()->state() != QMovie::NotRunning)
            copy.setDevicePixelRatio(label->devicePixelRatioF());
    }
    QProxyStyle::drawItemPixmap(painter, rect, alignment, copy);
}

void ApplicationStyle::polish(QWidget* w)
{
    if (qobject_cast<QMenu*>(w)
            || qobject_cast<QComboBoxPrivateContainer*>(w)
            || qobject_cast<QMdiSubWindow*>(w)) {

        if (!w->property("_q_ApplicationStyle_tinkered").isValid()) {
            w->setProperty("_q_ApplicationStyle_tinkered", true);
            for (QWidget* wd : w->findChildren<QWidget*>()) {
                if (wd != w)
                    wd->setStyleSheet(u"QWidget { background: transparent; }"_s);
            }
            class A final : public QObject {
                bool eventFilter(QObject* w, QEvent* e) override
                {
                    if (e->type() == QEvent::Show) {
                        QWidget* wid = static_cast<QWidget*>(w);
                        QWidget* parentWid = wid->parentWidget();
                        wid->setWindowFlags(wid->windowFlags()
                                            | Qt::BypassWindowManagerHint
                                            | Qt::NoDropShadowWindowHint
                                            | Qt::FramelessWindowHint);
#if defined(Q_OS_WINDOWS)
                        HWND hwnd = reinterpret_cast<HWND>(wid->winId());
                        DWORD style = GetClassLong(hwnd, GCL_STYLE);
                        style &= ~CS_DROPSHADOW;
                        SetClassLong(hwnd, GCL_STYLE, style);
#endif
                        wid->removeEventFilter(this);
                        if (parentWid)
                            wid->show();
                    }
                    return false;
                }
            } static shadowRemover;
            w->installEventFilter(&shadowRemover);
        }
        w->setAttribute(Qt::WA_TranslucentBackground, true);
        w->setAutoFillBackground(false);
    }

    QProxyStyle::polish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);
        w->setAttribute(Qt::WA_Hover, true);
        w->setMouseTracking(true);
    }
}

void ApplicationStyle::unpolish(QWidget* w)
{
    if (qobject_cast<QMenu*>(w) && !w->testAttribute(Qt::WA_SetPalette)) {
        QPalette pal = qApp->palette(w);
        w->setPalette(pal);
        w->setAttribute(Qt::WA_SetPalette, false);
        w->setWindowOpacity(1.0);
    }

    if (QComboBox *combo = qobject_cast<QComboBox*>(w)) {
        if (!combo->isEditable()) {
            if (QWidget *widget = combo->findChild<QComboBoxPrivateContainer*>())
                widget->setWindowOpacity(1.0);
        }
    }

    if (QFocusFrame *frame = qobject_cast<QFocusFrame*>(w))
        frame->setAttribute(Qt::WA_NoSystemBackground, true);

    QProxyStyle::unpolish(w);

    if (qobject_cast<QScrollBar*>(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
        w->setMouseTracking(false);
    }
}
