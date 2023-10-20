#include <muda/compute_graph/compute_graph.h>
#include <muda/compute_graph/compute_graph_node.h>

namespace muda
{
MUDA_INLINE void ComputeGraphVarBase::base_update()
{
    for(auto& [graph, info] : m_related_closure_infos)
    {
        graph->m_need_update = true;
        for(auto& id : info.closure_ids)
            graph->m_closure_need_update[id.value()] = true;
    }
    m_is_valid = true;
}
MUDA_INLINE void ComputeGraphVarBase::base_building_eval()
{
    _building_eval(ComputeGraphVarUsage::ReadWrite);
}

MUDA_INLINE void ComputeGraphVarBase::base_building_ceval() const
{
    _building_eval(ComputeGraphVarUsage::Read);
}

MUDA_INLINE void ComputeGraphVarBase::_building_eval(ComputeGraphVarUsage usage) const
{
    auto acc   = details::ComputeGraphAccessor();
    auto graph = ComputeGraphBuilder::instance().current_graph();
    m_related_closure_infos[graph].closure_ids.insert(graph->current_closure_id());
    graph->emplace_related_var(const_cast<ComputeGraphVarBase*>(this));
    acc.set_var_usage(var_id(), usage);
}

MUDA_INLINE void ComputeGraphVarBase::remove_related_closure_infos(ComputeGraph* graph)
{
    auto iter = m_related_closure_infos.find(graph);
    if(iter != m_related_closure_infos.end())
    {
        m_related_closure_infos.erase(iter);
    }
}

MUDA_INLINE void ComputeGraphVarBase::graphviz_def(std::ostream& o) const
{
    graphviz_id(o);
    o << "[";
    if(!name().empty())
        o << "label=\"" << name() << "\",";
    o << R"(shape="rectangle", color="#F08705", style="filled,rounded", fillcolor="#F5AF58"])";
}

MUDA_INLINE void ComputeGraphVarBase::graphviz_id(std::ostream& o) const
{
    o << "var_" << var_id();
}

MUDA_INLINE void ComputeGraphVarBase::update()
{
    this->base_update();
}

MUDA_INLINE Event::QueryResult ComputeGraphVarBase::query()
{
    for(auto& [graph, info] : m_related_closure_infos)
    {
        if(graph->query() == Event::QueryResult::eNotReady)
            return Event::QueryResult::eNotReady;
    }
    return Event::QueryResult::eFinished;
}

MUDA_INLINE bool ComputeGraphVarBase::is_using()
{
    return query() == Event::QueryResult::eNotReady;
}

MUDA_INLINE void ComputeGraphVarBase::sync()
{
    for(auto& [graph, info] : m_related_closure_infos)
        on().wait(graph->m_event);
}


// ComputeGraphVar<T>:

template <typename T>
MUDA_INLINE typename ComputeGraphVar<T>::RWViewer ComputeGraphVar<T>::eval()
{
    auto phase = ComputeGraphBuilder::current_phase();
    switch(phase)
    {
        case ComputeGraphPhase::None: {
            MUDA_ERROR_WITH_LOCATION("ComputeGraphVar.eval() is not allowed outside Graph Closure");
        }
        break;
        case ComputeGraphPhase::TopoBuilding:
        case ComputeGraphPhase::Building: {
            auto acc = details::ComputeGraphAccessor();
            acc.check_allow_var_eval();
            if(!acc.is_topo_built())
            {
                if constexpr(std::is_same_v<T, read_only_viewer_t<T>>)
                {
                    // they are all read only(e.g. host float/int ...)
                    this->base_building_ceval();
                }
                else
                {
                    this->base_building_eval();
                }
            }
        }
        break;
        case ComputeGraphPhase::Updating:
        default:  // nothing to do
            break;
    }
    return m_value;
}

template <typename T>
MUDA_INLINE typename ComputeGraphVar<T>::ROViewer ComputeGraphVar<T>::ceval() const
{
    auto phase = ComputeGraphBuilder::current_phase();
    switch(phase)
    {
        case ComputeGraphPhase::None: {
            MUDA_ERROR_WITH_LOCATION("ComputeGraphVar.eval() is not allowed outside Graph Closure");
        }
        break;
        case ComputeGraphPhase::TopoBuilding:
        case ComputeGraphPhase::Building: {
            this->base_building_ceval();
        }
        break;
        case ComputeGraphPhase::Updating: {
            // nothing to do
        }
        default:
            break;
    }
    return m_value;
}

template <typename T>
MUDA_INLINE void ComputeGraphVar<T>::update(const RWViewer& view)
{
    MUDA_ASSERT(!is_using(), "ComputeGraphVar is using, can't update");
    ComputeGraphVarBase::update();
    m_value = view;
}
template <typename T>
MUDA_INLINE ComputeGraphVar<T>& ComputeGraphVar<T>::operator=(const RWViewer& view)
{
    update(view);
    return *this;
}
}  // namespace muda