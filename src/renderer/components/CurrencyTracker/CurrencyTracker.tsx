import React, { useState, useEffect } from 'react';
import './CurrencyTracker.scss';

interface CurrencyRate {
  name: string;
  chaosEquivalent: number;
  change: number;
  icon?: string;
}

interface CurrencyTrackerProps {
  onClose?: () => void;
}

const CurrencyTracker: React.FC<CurrencyTrackerProps> = ({ onClose }) => {
  const [currencyRates, setCurrencyRates] = useState<CurrencyRate[]>([]);
  const [loading, setLoading] = useState(true);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);
  const [convertFrom, setConvertFrom] = useState('Chaos Orb');
  const [convertTo, setConvertTo] = useState('Divine Orb');
  const [convertAmount, setConvertAmount] = useState(1);
  const [convertResult, setConvertResult] = useState<number | null>(null);
  const [league, setLeague] = useState('Affliction');

  useEffect(() => {
    // Fetch currency data on component mount
    fetchCurrencyData();

    // Set up auto-refresh every 5 minutes
    const interval = setInterval(() => {
      fetchCurrencyData();
    }, 5 * 60 * 1000);

    return () => clearInterval(interval);
  }, [league]);

  const fetchCurrencyData = async () => {
    try {
      setLoading(true);

      // This would call the currency service via IPC in a real implementation
      // For now, using mock data
      const mockRates: CurrencyRate[] = [
        { name: 'Divine Orb', chaosEquivalent: 180, change: 2.5 },
        { name: 'Exalted Orb', chaosEquivalent: 20, change: -1.2 },
        { name: 'Chaos Orb', chaosEquivalent: 1, change: 0 },
        { name: 'Orb of Alchemy', chaosEquivalent: 0.2, change: 0.5 },
        { name: 'Orb of Alteration', chaosEquivalent: 0.1, change: 0 },
        { name: 'Chromatic Orb', chaosEquivalent: 0.05, change: 0 },
        { name: 'Jeweller\'s Orb', chaosEquivalent: 0.15, change: 0.1 },
        { name: 'Orb of Fusing', chaosEquivalent: 0.5, change: -0.3 },
        { name: 'Vaal Orb', chaosEquivalent: 1.5, change: 0.8 },
        { name: 'Gemcutter\'s Prism', chaosEquivalent: 2, change: 0.2 },
        { name: 'Cartographer\'s Chisel', chaosEquivalent: 0.3, change: 0 },
        { name: 'Orb of Regret', chaosEquivalent: 1.2, change: -0.1 },
      ];

      setCurrencyRates(mockRates);
      setLastUpdate(new Date());
      setLoading(false);
    } catch (error) {
      console.error('Error fetching currency data:', error);
      setLoading(false);
    }
  };

  const handleConvert = () => {
    const from = currencyRates.find(c => c.name === convertFrom);
    const to = currencyRates.find(c => c.name === convertTo);

    if (from && to) {
      const chaosValue = convertAmount * from.chaosEquivalent;
      const result = chaosValue / to.chaosEquivalent;
      setConvertResult(Math.round(result * 100) / 100);
    }
  };

  const formatChange = (change: number) => {
    const sign = change > 0 ? '+' : '';
    return `${sign}${change.toFixed(1)}%`;
  };

  return (
    <div className="currency-tracker">
      <div className="currency-tracker__header">
        <h3>Currency Exchange Rates</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>×</button>
        )}
      </div>

      <div className="currency-tracker__info">
        <span className="league">League: {league}</span>
        {lastUpdate && (
          <span className="last-update">
            Updated: {lastUpdate.toLocaleTimeString()}
          </span>
        )}
      </div>

      {loading ? (
        <div className="currency-tracker__loading">Loading currency data...</div>
      ) : (
        <>
          <div className="currency-tracker__rates">
            <div className="rates-header">
              <span>Currency</span>
              <span>Chaos Value</span>
              <span>Change 24h</span>
            </div>
            {currencyRates.map((rate) => (
              <div key={rate.name} className="rate-row">
                <span className="currency-name">{rate.name}</span>
                <span className="chaos-value">
                  {rate.chaosEquivalent >= 1
                    ? rate.chaosEquivalent.toFixed(1)
                    : rate.chaosEquivalent.toFixed(2)}c
                </span>
                <span className={`change ${rate.change > 0 ? 'positive' : rate.change < 0 ? 'negative' : 'neutral'}`}>
                  {formatChange(rate.change)}
                </span>
              </div>
            ))}
          </div>

          <div className="currency-tracker__converter">
            <h4>Currency Converter</h4>
            <div className="converter-inputs">
              <div className="input-group">
                <input
                  type="number"
                  value={convertAmount}
                  onChange={(e) => setConvertAmount(Number(e.target.value))}
                  min="0"
                  step="1"
                />
                <select
                  value={convertFrom}
                  onChange={(e) => setConvertFrom(e.target.value)}
                >
                  {currencyRates.map((rate) => (
                    <option key={rate.name} value={rate.name}>
                      {rate.name}
                    </option>
                  ))}
                </select>
              </div>

              <span className="arrow">→</span>

              <div className="input-group">
                <select
                  value={convertTo}
                  onChange={(e) => setConvertTo(e.target.value)}
                >
                  {currencyRates.map((rate) => (
                    <option key={rate.name} value={rate.name}>
                      {rate.name}
                    </option>
                  ))}
                </select>
              </div>

              <button onClick={handleConvert} className="convert-btn">
                Convert
              </button>
            </div>

            {convertResult !== null && (
              <div className="converter-result">
                {convertAmount} {convertFrom} = {convertResult} {convertTo}
              </div>
            )}
          </div>

          <div className="currency-tracker__actions">
            <button onClick={fetchCurrencyData} className="refresh-btn">
              Refresh Data
            </button>
          </div>
        </>
      )}
    </div>
  );
};

export default CurrencyTracker;
