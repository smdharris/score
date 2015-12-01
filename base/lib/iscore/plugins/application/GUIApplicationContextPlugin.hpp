#pragma once
#include <iscore/tools/NamedObject.hpp>
#include <qnamespace.h>
#include <QString>
#include <vector>

#include <core/application/ApplicationContext.hpp>

class QAction;
class QObject;
namespace iscore {
class Document;
struct OrderedToolbar;
}  // namespace iscore
struct VisitorVariant;

namespace iscore
{
    class Application;
    class DocumentPluginModel;
    class MenubarManager;

    /**
     * @brief The GUIApplicationContextPlugin class
     *
     * This class's goal is to :
     * * Instantiate some elements that are deeply intertwined with Qt : menus, toolbars
     * * Manage the Commands of the plug-in : it has to be able to instantiate any meaningful
     *   Command, if received by the network.
     *
     * It is instatiated exactly once.
     */
    class GUIApplicationContextPlugin
    {
            // Menus : trouver un moyen pour créer automatiquement si n'existe pas ?
            // Dire chemin : File/Export/SomeCustomExport ?
            // Pb. : traduction ? (ex. : soft traduit & plug pas traduit ?)
            // Fournir menus de base : Fichier Edition Affichage Objet Arrangement Devices Fenêtre Paramètres Aide

        public:
            GUIApplicationContextPlugin(iscore::Application& presenter,
                                   const QString& name,
                                   QObject* parent);

            virtual ~GUIApplicationContextPlugin();

            virtual void populateMenus(iscore::MenubarManager*);
            virtual std::vector<iscore::OrderedToolbar> makeToolbars();
            virtual std::vector<QAction*> actions();

            virtual DocumentPluginModel* loadDocumentPlugin(
                    const QString& name,
                    const VisitorVariant& var,
                    iscore::Document *parent);

            const ApplicationContext& context() const;
            Document* currentDocument() const;

            virtual void on_newDocument(iscore::Document* doc);
            virtual void on_loadedDocument(iscore::Document* doc);
            virtual void prepareNewDocument();

            virtual void on_documentChanged(
                    iscore::Document* olddoc,
                    iscore::Document* newdoc);

        private:
            ApplicationContext m_appContext;
    };

}
