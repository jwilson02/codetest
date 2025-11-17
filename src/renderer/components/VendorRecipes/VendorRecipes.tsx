import React, { useState, useEffect, useRef } from 'react';
import './VendorRecipes.scss';

interface Recipe {
  id: string;
  name: string;
  category: string;
  result: string;
  requirements: string[];
  notes: string;
  profitability: string;
  difficulty: string;
  tags: string[];
}

interface VendorRecipesProps {
  onClose?: () => void;
}

const VendorRecipes: React.FC<VendorRecipesProps> = ({ onClose }) => {
  const [recipes, setRecipes] = useState<Recipe[]>([]);
  const [filteredRecipes, setFilteredRecipes] = useState<Recipe[]>([]);
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedCategory, setSelectedCategory] = useState('all');
  const [categories, setCategories] = useState<string[]>([]);
  const [favorites, setFavorites] = useState<Set<string>>(new Set());
  const [expandedRecipe, setExpandedRecipe] = useState<string | null>(null);
  const [showFavoritesOnly, setShowFavoritesOnly] = useState(false);

  const recipeServiceRef = useRef<any>(null);

  useEffect(() => {
    // Initialize vendor recipe service
    const VendorRecipeService = window.require(
      '../services/vendorRecipeService.js'
    );
    recipeServiceRef.current = new VendorRecipeService();

    // Load recipes and categories
    loadRecipes();
    loadCategories();
    loadFavorites();
  }, []);

  useEffect(() => {
    // Filter recipes when search or category changes
    filterRecipes();
  }, [searchQuery, selectedCategory, recipes, showFavoritesOnly, favorites]);

  const loadRecipes = () => {
    if (recipeServiceRef.current) {
      const allRecipes = recipeServiceRef.current.getAllRecipes();
      setRecipes(allRecipes);
    }
  };

  const loadCategories = () => {
    if (recipeServiceRef.current) {
      const cats = recipeServiceRef.current.getCategories();
      setCategories(['all', ...cats]);
    }
  };

  const loadFavorites = () => {
    const stored = localStorage.getItem('poe-vendor-favorites');
    if (stored) {
      try {
        const favs = JSON.parse(stored);
        setFavorites(new Set(favs));

        // Update service favorites
        if (recipeServiceRef.current) {
          recipeServiceRef.current.importFavorites(stored);
        }
      } catch (error) {
        console.error('Error loading favorites:', error);
      }
    }
  };

  const saveFavorites = (favs: Set<string>) => {
    const favsArray = Array.from(favs);
    localStorage.setItem('poe-vendor-favorites', JSON.stringify(favsArray));
  };

  const filterRecipes = () => {
    if (!recipeServiceRef.current) {
      return;
    }

    let filtered = recipes;

    // Filter by category
    if (selectedCategory !== 'all') {
      filtered = recipeServiceRef.current.getRecipesByCategory(selectedCategory);
    }

    // Filter by search query
    if (searchQuery) {
      filtered = recipeServiceRef.current.searchRecipes(searchQuery);

      // Also apply category filter if both are set
      if (selectedCategory !== 'all') {
        filtered = filtered.filter(r => r.category === selectedCategory);
      }
    }

    // Filter by favorites
    if (showFavoritesOnly) {
      filtered = filtered.filter(r => favorites.has(r.id));
    }

    setFilteredRecipes(filtered);
  };

  const toggleFavorite = (recipeId: string) => {
    const newFavorites = new Set(favorites);

    if (newFavorites.has(recipeId)) {
      newFavorites.delete(recipeId);
    } else {
      newFavorites.add(recipeId);
    }

    setFavorites(newFavorites);
    saveFavorites(newFavorites);

    if (recipeServiceRef.current) {
      recipeServiceRef.current.toggleFavorite(recipeId);
    }
  };

  const toggleExpanded = (recipeId: string) => {
    setExpandedRecipe(expandedRecipe === recipeId ? null : recipeId);
  };

  const getProfitabilityColor = (profitability: string) => {
    const colors: { [key: string]: string } = {
      'very-high': '#ffd700',
      'high': '#4caf50',
      'medium': '#2196f3',
      'low': '#666666',
      'situational': '#9c27b0',
      'high-risk': '#f44336',
    };

    return colors[profitability] || '#666666';
  };

  const getDifficultyColor = (difficulty: string) => {
    const colors: { [key: string]: string } = {
      'easy': '#4caf50',
      'medium': '#ff9800',
      'hard': '#d32f2f',
    };

    return colors[difficulty] || '#666666';
  };

  const formatCategoryName = (category: string) => {
    return category.charAt(0).toUpperCase() + category.slice(1);
  };

  const getValuableRecipes = () => {
    if (!recipeServiceRef.current) return [];
    return recipeServiceRef.current.getValuableRecipes();
  };

  return (
    <div className="vendor-recipes">
      <div className="vendor-recipes-header">
        <h3>Vendor Recipes Database</h3>
        {onClose && (
          <button className="close-btn" onClick={onClose}>
            ×
          </button>
        )}
      </div>

      <div className="search-section">
        <input
          type="text"
          className="search-input"
          placeholder="Search recipes, items, or results..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
        />
      </div>

      <div className="filter-section">
        <div className="category-filters">
          {categories.map((category) => (
            <button
              key={category}
              className={`category-btn ${selectedCategory === category ? 'active' : ''}`}
              onClick={() => setSelectedCategory(category)}
            >
              {formatCategoryName(category)}
            </button>
          ))}
        </div>

        <div className="view-options">
          <button
            className={`favorites-toggle ${showFavoritesOnly ? 'active' : ''}`}
            onClick={() => setShowFavoritesOnly(!showFavoritesOnly)}
          >
            {showFavoritesOnly ? '★' : '☆'} Favorites ({favorites.size})
          </button>
        </div>
      </div>

      <div className="recipes-stats">
        <span>Showing {filteredRecipes.length} recipes</span>
        <span className="valuable-count">
          {getValuableRecipes().length} valuable recipes
        </span>
      </div>

      <div className="recipes-list">
        {filteredRecipes.length > 0 ? (
          filteredRecipes.map((recipe) => (
            <div
              key={recipe.id}
              className={`recipe-card ${expandedRecipe === recipe.id ? 'expanded' : ''}`}
            >
              <div className="recipe-header" onClick={() => toggleExpanded(recipe.id)}>
                <div className="recipe-title">
                  <button
                    className="favorite-btn"
                    onClick={(e) => {
                      e.stopPropagation();
                      toggleFavorite(recipe.id);
                    }}
                  >
                    {favorites.has(recipe.id) ? '★' : '☆'}
                  </button>
                  <h4>{recipe.name}</h4>
                  <span className="category-badge">{recipe.category}</span>
                </div>
                <div className="recipe-indicators">
                  <span
                    className="profitability-indicator"
                    style={{ color: getProfitabilityColor(recipe.profitability) }}
                    title={`Profitability: ${recipe.profitability}`}
                  >
                    💰
                  </span>
                  <span
                    className="difficulty-indicator"
                    style={{ color: getDifficultyColor(recipe.difficulty) }}
                    title={`Difficulty: ${recipe.difficulty}`}
                  >
                    {recipe.difficulty === 'easy' ? '●' : recipe.difficulty === 'medium' ? '●●' : '●●●'}
                  </span>
                  <span className="expand-icon">
                    {expandedRecipe === recipe.id ? '▼' : '▶'}
                  </span>
                </div>
              </div>

              <div className="recipe-result">
                <strong>Result:</strong> {recipe.result}
              </div>

              {expandedRecipe === recipe.id && (
                <div className="recipe-details">
                  <div className="requirements-section">
                    <strong>Requirements:</strong>
                    <ul>
                      {recipe.requirements.map((req, index) => (
                        <li key={index}>{req}</li>
                      ))}
                    </ul>
                  </div>

                  {recipe.notes && (
                    <div className="notes-section">
                      <strong>Notes:</strong>
                      <p>{recipe.notes}</p>
                    </div>
                  )}

                  <div className="meta-info">
                    <div className="meta-item">
                      <span className="meta-label">Profitability:</span>
                      <span
                        className="meta-value"
                        style={{ color: getProfitabilityColor(recipe.profitability) }}
                      >
                        {recipe.profitability}
                      </span>
                    </div>
                    <div className="meta-item">
                      <span className="meta-label">Difficulty:</span>
                      <span
                        className="meta-value"
                        style={{ color: getDifficultyColor(recipe.difficulty) }}
                      >
                        {recipe.difficulty}
                      </span>
                    </div>
                  </div>

                  {recipe.tags.length > 0 && (
                    <div className="tags-section">
                      {recipe.tags.map((tag, index) => (
                        <span key={index} className="tag">
                          {tag}
                        </span>
                      ))}
                    </div>
                  )}
                </div>
              )}
            </div>
          ))
        ) : (
          <div className="empty-state">
            <p>No recipes found</p>
            {searchQuery && (
              <p className="hint">Try adjusting your search or filter</p>
            )}
          </div>
        )}
      </div>

      <div className="quick-reference">
        <h4>Quick Reference - Most Valuable</h4>
        <div className="valuable-recipes">
          {getValuableRecipes().slice(0, 5).map((recipe) => (
            <div
              key={recipe.id}
              className="valuable-recipe-item"
              onClick={() => {
                setSearchQuery(recipe.name);
                setExpandedRecipe(recipe.id);
              }}
            >
              <span className="recipe-name">{recipe.name}</span>
              <span className="recipe-result">{recipe.result}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default VendorRecipes;
