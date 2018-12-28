#pragma once

#include <possumwood_sdk/editor.h>

#include <QPlainTextEdit>
#include <QHBoxLayout>

namespace possumwood {

class SourceEditor : public possumwood::Editor {
	public:
		SourceEditor(dependency_graph::InAttr<std::string>& src);
		virtual ~SourceEditor();

		virtual QWidget* widget() override;

	protected:
		QHBoxLayout* buttonsLayout() const;
		QPlainTextEdit* editorWidget() const;

		virtual void valueChanged(const dependency_graph::Attr& attr) override;

	private:
		dependency_graph::InAttr<std::string>* m_src;

		QWidget* m_widget;
		QHBoxLayout* m_buttonsLayout;

		QPlainTextEdit* m_editor;

		bool m_blockedSignals;
};

}
