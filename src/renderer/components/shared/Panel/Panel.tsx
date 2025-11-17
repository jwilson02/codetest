import React, { useState } from 'react';
import Draggable, { DraggableData, DraggableEvent } from 'react-draggable';
import { ResizableBox } from 'react-resizable';
import { useOverlay } from '@contexts/OverlayContext';
import { Position, Size } from '@types/index';
import './Panel.scss';
import 'react-resizable/css/styles.css';

interface PanelProps {
  id: string;
  title: string;
  children: React.ReactNode;
  className?: string;
  resizable?: boolean;
  minWidth?: number;
  minHeight?: number;
  maxWidth?: number;
  maxHeight?: number;
}

const Panel: React.FC<PanelProps> = ({
  id,
  title,
  children,
  className = '',
  resizable = true,
  minWidth = 250,
  minHeight = 200,
  maxWidth = 1200,
  maxHeight = 900,
}) => {
  const {
    settings,
    togglePanel,
    updatePanelPosition,
    updatePanelSize,
    togglePanelMinimize
  } = useOverlay();

  const panel = settings.panels[id];
  const [isDragging, setIsDragging] = useState(false);

  const handleDragStart = () => {
    setIsDragging(true);
  };

  const handleDragStop = (_e: DraggableEvent, data: DraggableData) => {
    setIsDragging(false);
    const newPosition: Position = { x: data.x, y: data.y };
    updatePanelPosition(id, newPosition);
  };

  const handleResize = (_e: any, { size }: any) => {
    const newSize: Size = { width: size.width, height: size.height };
    updatePanelSize(id, newSize);
  };

  const handleClose = () => {
    togglePanel(id);
  };

  const handleMinimize = () => {
    togglePanelMinimize(id);
  };

  const panelContent = (
    <div className={`panel ${className} ${isDragging ? 'dragging' : ''} ${panel.minimized ? 'minimized' : ''}`}>
      <div className="panel-header">
        <div className="panel-title">{title}</div>
        <div className="panel-controls">
          <button
            className="panel-btn minimize-btn"
            onClick={handleMinimize}
            title={panel.minimized ? 'Restore' : 'Minimize'}
          >
            {panel.minimized ? '□' : '_'}
          </button>
          <button
            className="panel-btn close-btn"
            onClick={handleClose}
            title="Close"
          >
            ×
          </button>
        </div>
      </div>
      {!panel.minimized && (
        <div className="panel-content">
          {children}
        </div>
      )}
    </div>
  );

  return (
    <Draggable
      handle=".panel-header"
      position={panel.position}
      onStart={handleDragStart}
      onStop={handleDragStop}
      bounds="parent"
    >
      {resizable && !panel.minimized ? (
        <div style={{ position: 'absolute' }}>
          <ResizableBox
            width={panel.size.width}
            height={panel.size.height}
            minConstraints={[minWidth, minHeight]}
            maxConstraints={[maxWidth, maxHeight]}
            onResize={handleResize}
            resizeHandles={['se', 'sw', 'ne', 'nw', 's', 'e', 'w', 'n']}
          >
            {panelContent}
          </ResizableBox>
        </div>
      ) : (
        <div style={{ position: 'absolute', ...panel.size }}>
          {panelContent}
        </div>
      )}
    </Draggable>
  );
};

export default Panel;
