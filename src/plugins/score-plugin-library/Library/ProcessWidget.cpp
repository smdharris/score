#include "ProcessWidget.hpp"
#include <Library/ProcessesItemModel.hpp>
#include <Library/PresetItemModel.hpp>
#include <Library/RecursiveFilterProxy.hpp>
#include <Library/ItemModelFilterLineEdit.hpp>
#include <Library/LibraryWidget.hpp>

#include <Process/ProcessList.hpp>

#include <score/widgets/MarginLess.hpp>
#include <score/application/GUIApplicationContext.hpp>

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>


namespace Library
{
class InfoWidget final : public QScrollArea
{
public:
  InfoWidget(QWidget* parent)
  {
    setMinimumHeight(120);
    setMaximumHeight(160);
    auto lay = new QVBoxLayout{this};
    QFont f;
    f.setBold(true);
    m_name.setFont(f);
    lay->addWidget(&m_name);
    lay->addWidget(&m_author);
    m_author.setWordWrap(true);
    lay->addWidget(&m_io);
    lay->addWidget(&m_description);
    m_description.setWordWrap(true);
    lay->addWidget(&m_tags);
    m_tags.setWordWrap(true);
    setVisible(false);
  }

  void setData(const optional<ProcessData>& d)
  {
    if (d)
    {
      if (auto f = score::GUIAppContext()
                       .interfaces<Process::ProcessFactoryList>()
                       .get(d->key))
      {
        setVisible(true);
        auto desc = f->descriptor(QString{/*TODO pass customdata ?*/});
        m_name.setText(desc.prettyName);
        m_author.setText(tr("Provided by ") + desc.author);
        m_description.setText(desc.description);
        QString io;
        if (desc.inlets)
        {
          io += QString::number(desc.inlets->size()) + tr(" input");
          if (desc.inlets->size() > 1)
            io += "s";
        }
        if (desc.inlets && desc.outlets)
          io += ", ";
        if (desc.outlets)
        {
          io += QString::number(desc.outlets->size()) + tr(" output");
          if (desc.outlets->size() > 1)
            io += "s";
        }
        m_io.setText(io);
        if (!desc.tags.empty())
        {
          m_tags.setText(tr("Tags: ") + desc.tags.join(", "));
        }
        else
        {
          m_tags.clear();
        }
        return;
      }
    }

    setVisible(false);
  }

  QLabel m_name;
  QLabel m_io;
  QLabel m_description;
  QLabel m_author;
  QLabel m_tags;
};

ProcessWidget::ProcessWidget(
    const score::GUIApplicationContext& ctx,
    QWidget* parent)
    : QWidget{parent}
    , m_processModel{new ProcessesItemModel{ctx, this}}
    , m_presetModel{new PresetItemModel{ctx, this}}
    , m_split{Qt::Vertical, this}
{
  auto slay = new score::MarginLess<QVBoxLayout>{this};
  slay->addWidget(&m_split);
  setStatusTip(QObject::tr("This panel shows the available processes.\n"
                           "They can be drag'n'dropped in the score, in intervals, "
                           "and sometimes in effect chains."));
  auto top_w = new QWidget;
  auto lay = new score::MarginLess<QVBoxLayout>{top_w};

  {
    auto processFilterProxy = new RecursiveFilterProxy{this};
    processFilterProxy->setSourceModel(m_processModel);
    processFilterProxy->setFilterKeyColumn(0);
    lay->addWidget(new ItemModelFilterLineEdit{*processFilterProxy, m_tv, this});
    lay->addWidget(&m_tv);
    m_tv.setModel(processFilterProxy);
    m_tv.setStatusTip(statusTip());
    setup_treeview(m_tv);
  }

  auto presetFilterProxy = new PresetFilterProxy{this};
  {
    presetFilterProxy->setSourceModel(m_presetModel);
    m_lv.setModel(presetFilterProxy);
    m_lv.setAcceptDrops(true);
    m_lv.setDragEnabled(true);
  }

  auto infoWidg = new InfoWidget{this};
  infoWidg->setStatusTip(statusTip());

  connect(&m_tv, &ProcessTreeView::selected, this, [=](const auto& pdata) {
    infoWidg->setData(pdata);
    if(pdata)
      presetFilterProxy->currentFilter = pdata->key;
    else
      presetFilterProxy->currentFilter = {};
    presetFilterProxy->invalidate();
  });

  m_split.addWidget(top_w);
  m_split.addWidget(&m_lv);
  m_split.addWidget(infoWidg);
  top_w->setMinimumHeight(100);
  m_lv.setMinimumHeight(100);
  m_split.setCollapsible(0, false);
  m_split.setCollapsible(1, false);
  m_split.setCollapsible(2, false);
  m_split.setStretchFactor(0, 3);
  m_split.setStretchFactor(1, 1);
  m_split.setStretchFactor(2, 1);
}

ProcessWidget::~ProcessWidget()
{

}

}
