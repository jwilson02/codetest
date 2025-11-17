import React, { useState, useEffect, useRef } from 'react';
import './TradeWhisperManager.scss';

interface Trade {
  id: string;
  type: 'standard' | 'asynchronous' | 'bulk';
  playerName: string;
  item?: string;
  price?: string;
  league?: string;
  wantItem?: string;
  offerItem?: string;
  stashTab?: string | null;
  stashLeft?: number | null;
  stashTop?: number | null;
  status: 'pending' | 'completed' | 'cancelled';
  timestamp: number;
  addedAt: number;
  completedAt?: number | null;
}

interface Template {
  id: string;
  name: string;
  message: string;
  shortcut?: string | null;
  custom?: boolean;
}

interface Statistics {
  totalTrades: number;
  completedTrades: number;
  cancelledTrades: number;
  sessionStart: number;
  tradesPerHour: number;
}

interface TradeWhisperManagerProps {
  onClose?: () => void;
}

const TradeWhisperManager: React.FC<TradeWhisperManagerProps> = ({ onClose }) => {
  const [trades, setTrades] = useState<Trade[]>([]);
  const [templates, setTemplates] = useState<Template[]>([]);
  const [statistics, setStatistics] = useState<Statistics>({
    totalTrades: 0,
    completedTrades: 0,
    cancelledTrades: 0,
    sessionStart: Date.now(),
    tradesPerHour: 0,
  });
  const [whisperInput, setWhisperInput] = useState('');
  const [activeTab, setActiveTab] = useState<'active' | 'history'>('active');
  const [customTemplateName, setCustomTemplateName] = useState('');
  const [customTemplateMessage, setCustomTemplateMessage] = useState('');
  const [showTemplateEditor, setShowTemplateEditor] = useState(false);
  const [copiedResponse, setCopiedResponse] = useState<string | null>(null);

  const tradeServiceRef = useRef<any>(null);

  useEffect(() => {
    // Initialize trade whisper service
    const TradeWhisperService = window.require(
      '../services/tradeWhisperService.js'
    );
    tradeServiceRef.current = new TradeWhisperService();

    // Load templates
    loadTemplates();

    // Load initial trades from localStorage
    loadTradesFromStorage();

    // Set up clipboard monitoring (simplified - would use IPC in real app)
    const interval = setInterval(() => {
      updateStatistics();
    }, 5000);

    return () => clearInterval(interval);
  }, []);

  const loadTemplates = () => {
    if (tradeServiceRef.current) {
      const loadedTemplates = tradeServiceRef.current.getTemplates();
      setTemplates(loadedTemplates);
    }
  };

  const loadTradesFromStorage = () => {
    const stored = localStorage.getItem('poe-trade-whispers');
    if (stored && tradeServiceRef.current) {
      tradeServiceRef.current.importTrades(stored);
      updateTradesAndStats();
    }
  };

  const saveToStorage = () => {
    if (tradeServiceRef.current) {
      const data = tradeServiceRef.current.exportTrades();
      localStorage.setItem('poe-trade-whispers', data);
    }
  };

  const updateTradesAndStats = () => {
    if (tradeServiceRef.current) {
      const allTrades = tradeServiceRef.current.getAllTrades();
      setTrades(allTrades);
      updateStatistics();
    }
  };

  const updateStatistics = () => {
    if (tradeServiceRef.current) {
      const stats = tradeServiceRef.current.getStatistics();
      setStatistics(stats);
    }
  };

  const handleAddWhisper = () => {
    if (!whisperInput.trim() || !tradeServiceRef.current) {
      return;
    }

    const trade = tradeServiceRef.current.addTrade(whisperInput);

    if (trade) {
      updateTradesAndStats();
      saveToStorage();
      setWhisperInput('');
    } else {
      alert('Could not parse whisper. Please check the format.');
    }
  };

  const handleCompleteTrade = (tradeId: string) => {
    if (tradeServiceRef.current) {
      tradeServiceRef.current.completeTrade(tradeId);
      updateTradesAndStats();
      saveToStorage();
    }
  };

  const handleCancelTrade = (tradeId: string) => {
    if (tradeServiceRef.current) {
      tradeServiceRef.current.cancelTrade(tradeId);
      updateTradesAndStats();
      saveToStorage();
    }
  };

  const handleRemoveTrade = (tradeId: string) => {
    if (tradeServiceRef.current) {
      tradeServiceRef.current.removeTrade(tradeId);
      updateTradesAndStats();
      saveToStorage();
    }
  };

  const handleResponseClick = (tradeId: string, templateId: string) => {
    if (tradeServiceRef.current) {
      const response = tradeServiceRef.current.generateResponse(tradeId, templateId);

      if (response) {
        // Copy to clipboard
        navigator.clipboard.writeText(response);
        setCopiedResponse(`${tradeId}-${templateId}`);

        setTimeout(() => {
          setCopiedResponse(null);
        }, 2000);
      }
    }
  };

  const handleAddCustomTemplate = () => {
    if (!customTemplateName.trim() || !customTemplateMessage.trim()) {
      return;
    }

    if (tradeServiceRef.current) {
      tradeServiceRef.current.addTemplate(customTemplateName, customTemplateMessage);
      loadTemplates();
      setCustomTemplateName('');
      setCustomTemplateMessage('');
      setShowTemplateEditor(false);
    }
  };

  const handleClearHistory = () => {
    if (confirm('Clear all trade history? Active trades will be kept.')) {
      if (tradeServiceRef.current) {
        tradeServiceRef.current.clearHistory();
        updateTradesAndStats();
        saveToStorage();
      }
    }
  };

  const formatTimestamp = (timestamp: number) => {
    const date = new Date(timestamp);
    return date.toLocaleTimeString();
  };

  const formatDuration = (startTime: number, endTime: number | null | undefined) => {
    const duration = (endTime || Date.now()) - startTime;
    const seconds = Math.floor(duration / 1000);
    const minutes = Math.floor(seconds / 60);

    if (minutes > 0) {
      return `${minutes}m ${seconds % 60}s`;
    }
    return `${seconds}s`;
  };

  const getActiveTrades = () => {
    return trades.filter(trade => trade.status === 'pending');
  };

  const getHistoryTrades = () => {
    return trades.filter(trade => trade.status !== 'pending');
  };

  const renderTradeItem = (trade: Trade) => {
    const isActive = trade.status === 'pending';

    return (
      <div key={trade.id} className={`trade-item ${trade.type} ${trade.status}`}>
        <div className="trade-header">
          <div className="trade-info">
            <span className="player-name">@{trade.playerName}</span>
            <span className="trade-time">{formatTimestamp(trade.timestamp)}</span>
            {trade.type === 'asynchronous' && (
              <span className="async-badge">Async</span>
            )}
          </div>
          {!isActive && (
            <button
              className="remove-btn"
              onClick={() => handleRemoveTrade(trade.id)}
              title="Remove from history"
            >
              ×
            </button>
          )}
        </div>

        <div className="trade-details">
          {trade.type === 'bulk' ? (
            <div className="bulk-trade">
              <span className="want">Want: {trade.wantItem}</span>
              <span className="offer">Offer: {trade.offerItem}</span>
            </div>
          ) : (
            <>
              <div className="item-name">{trade.item}</div>
              <div className="price">{trade.price}</div>
              {trade.stashTab && (
                <div className="stash-location">
                  Stash: "{trade.stashTab}" ({trade.stashLeft}, {trade.stashTop})
                </div>
              )}
            </>
          )}
        </div>

        {isActive && (
          <div className="trade-actions">
            <div className="quick-responses">
              {templates.slice(0, 6).map((template) => (
                <button
                  key={template.id}
                  className={`response-btn ${
                    copiedResponse === `${trade.id}-${template.id}` ? 'copied' : ''
                  }`}
                  onClick={() => handleResponseClick(trade.id, template.id)}
                  title={template.message}
                >
                  {template.shortcut || template.name}
                </button>
              ))}
            </div>
            <div className="status-actions">
              <button
                className="complete-btn"
                onClick={() => handleCompleteTrade(trade.id)}
              >
                Complete
              </button>
              <button
                className="cancel-btn"
                onClick={() => handleCancelTrade(trade.id)}
              >
                Cancel
              </button>
            </div>
          </div>
        )}

        {!isActive && (
          <div className="trade-status-footer">
            <span className={`status-badge ${trade.status}`}>
              {trade.status.toUpperCase()}
            </span>
            <span className="duration">
              {formatDuration(trade.addedAt, trade.completedAt)}
            </span>
          </div>
        )}
      </div>
    );
  };

  return (
    <div className="trade-whisper-manager">
      <div className="trade-whisper-header">
        <h3>Trade Whisper Manager</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>
            ×
          </button>
        )}
      </div>

      <div className="statistics-bar">
        <div className="stat-item">
          <span className="stat-label">Active</span>
          <span className="stat-value">{getActiveTrades().length}</span>
        </div>
        <div className="stat-item">
          <span className="stat-label">Completed</span>
          <span className="stat-value">{statistics.completedTrades}</span>
        </div>
        <div className="stat-item">
          <span className="stat-label">Trades/Hour</span>
          <span className="stat-value">{statistics.tradesPerHour}</span>
        </div>
      </div>

      <div className="whisper-input-section">
        <textarea
          className="whisper-input"
          placeholder="Paste trade whisper here..."
          value={whisperInput}
          onChange={(e) => setWhisperInput(e.target.value)}
          rows={3}
        />
        <button className="add-whisper-btn" onClick={handleAddWhisper}>
          Add Whisper
        </button>
      </div>

      <div className="tabs">
        <button
          className={`tab ${activeTab === 'active' ? 'active' : ''}`}
          onClick={() => setActiveTab('active')}
        >
          Active Trades ({getActiveTrades().length})
        </button>
        <button
          className={`tab ${activeTab === 'history' ? 'active' : ''}`}
          onClick={() => setActiveTab('history')}
        >
          History ({getHistoryTrades().length})
        </button>
      </div>

      <div className="trades-list">
        {activeTab === 'active' ? (
          getActiveTrades().length > 0 ? (
            getActiveTrades().map(renderTradeItem)
          ) : (
            <div className="empty-state">No active trades</div>
          )
        ) : (
          <>
            {getHistoryTrades().length > 0 ? (
              <>
                {getHistoryTrades().map(renderTradeItem)}
                <button className="clear-history-btn" onClick={handleClearHistory}>
                  Clear History
                </button>
              </>
            ) : (
              <div className="empty-state">No trade history</div>
            )}
          </>
        )}
      </div>

      <div className="templates-section">
        <div className="section-header">
          <h4>Response Templates</h4>
          <button
            className="add-template-btn"
            onClick={() => setShowTemplateEditor(!showTemplateEditor)}
          >
            {showTemplateEditor ? 'Cancel' : '+ Add'}
          </button>
        </div>

        {showTemplateEditor && (
          <div className="template-editor">
            <input
              type="text"
              placeholder="Template name"
              value={customTemplateName}
              onChange={(e) => setCustomTemplateName(e.target.value)}
            />
            <input
              type="text"
              placeholder="Message"
              value={customTemplateMessage}
              onChange={(e) => setCustomTemplateMessage(e.target.value)}
            />
            <button onClick={handleAddCustomTemplate}>Save Template</button>
          </div>
        )}

        <div className="templates-list">
          {templates.map((template) => (
            <div key={template.id} className="template-item">
              <span className="template-name">{template.name}</span>
              <span className="template-message">{template.message}</span>
              {template.shortcut && (
                <span className="template-shortcut">{template.shortcut}</span>
              )}
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default TradeWhisperManager;
