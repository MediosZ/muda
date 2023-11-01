namespace muda
{
MUDA_INLINE MUDA_HOST GraphViewer::GraphViewer(cudaGraphExec_t graph)
    : m_graph(graph)
{
}
MUDA_INLINE MUDA_HOST void GraphViewer::launch(cudaStream_t stream) const
{
    auto graph_viewer_error_code = cudaGraphLaunch(m_graph, stream);
    if(graph_viewer_error_code != cudaSuccess)
    {
        auto error_string = cudaGetErrorString(graph_viewer_error_code);
        MUDA_KERNEL_ERROR_WITH_LOCATION(
            "GraphViewer[%s:%s]: launch error: %s", kernel_name(), name(), error_string);
    }
}
}  // namespace muda
