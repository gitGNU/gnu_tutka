#include <QCheckBox>
#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return new QCheckBox(parent);
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    static_cast<QCheckBox*>(editor)->setChecked(index.model()->data(index, Qt::EditRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, static_cast<QCheckBox*>(editor)->isChecked(), Qt::EditRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index)
}
