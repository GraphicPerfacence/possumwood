#pragma once

#include <string>

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "port.h"

namespace node_editor {

class Node : public QGraphicsRectItem {
	public:
		struct PortDefinition {
			QString name;
			Port::Type type;
			QColor color;
		};

		Node(const QString& name, const QPointF& position = QPointF(0, 0),
		     const std::initializer_list<PortDefinition>& ports = std::initializer_list<PortDefinition>());
		virtual ~Node();


		const QString name() const;

		unsigned portCount() const;
		Port& port(unsigned i);

		void addPort(const PortDefinition& def);
		void removePort(Port& p);

	protected:
		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

	private:
		void updateRect();

		QGraphicsRectItem* m_titleBackground;
		QGraphicsTextItem* m_title;
		QVector<Port*> m_ports;
};

}