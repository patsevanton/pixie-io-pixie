#pragma once
#include <memory>
#include <string>
#include <vector>

#include "src/carnot/planner/compiler_state/compiler_state.h"
#include "src/carnot/planner/objects/funcobject.h"
#include "src/carnot/planner/probes/probes.h"

namespace pl {
namespace carnot {
namespace planner {
namespace compiler {

/**
 * @brief TracingVariable holds the reference for a variable used in tracing (ie argument, return
 * value or latency).
 *
 */
class TracingVariableObject : public QLObject {
 public:
  static constexpr TypeDescriptor TracingVariableObjectType = {
      /* name */ "Tracing Variable",
      /* type */ QLObjectType::kTracingVariable,
  };

  static bool IsTracingVariable(const QLObjectPtr& ptr) {
    return ptr->type() == TracingVariableObjectType.type();
  }

  // The reference for this tracing variable.
  const std::string& id() const { return id_; }

  explicit TracingVariableObject(ASTVisitor* visitor, const std::string& id)
      : QLObject(TracingVariableObjectType, visitor), id_(id) {}

 private:
  std::string id_;
};

/**
 * @brief ProbeObject is the QLObject that wraps a probe.
 *
 */
class ProbeObject : public QLObject {
 public:
  static constexpr TypeDescriptor ProbeObjectType = {
      /* name */ "probe",
      /* type */ QLObjectType::kProbe,
  };

  static StatusOr<std::shared_ptr<ProbeObject>> Create(ASTVisitor* visitor,
                                                       const std::shared_ptr<TracepointIR>& probe);

  static bool IsProbe(const QLObjectPtr& ptr) { return ptr->type() == ProbeObjectType.type(); }
  std::shared_ptr<TracepointIR> probe() const { return probe_; }

 private:
  ProbeObject(ASTVisitor* visitor, const std::shared_ptr<TracepointIR>& probe)
      : QLObject(ProbeObjectType, visitor), probe_(probe) {}

  std::shared_ptr<TracepointIR> probe_;
};

class TraceModule : public QLObject {
 public:
  static constexpr TypeDescriptor TraceModuleType = {
      /* name */ "pxtrace",
      /* type */ QLObjectType::kTraceModule,
  };
  static StatusOr<std::shared_ptr<TraceModule>> Create(MutationsIR* mutations_ir,
                                                       ASTVisitor* ast_visitor);

  // Constant for the modules.
  inline static constexpr char kTraceModuleObjName[] = "pxtrace";

  // Constants for functions of pxtrace.
  inline static constexpr char kArgExprID[] = "ArgExpr";
  inline static constexpr char kArgExprDocstring[] = R"doc(
  Specifies a function argument to trace.

  Extracts the function argument, as specified by the provided expression.
  Traceable types are base types (`int`, `float`, etc.), strings and byte arrays.
  Base-type arguments are specified directly (`arg1`), while struct members are
  accessed using dotted notation (`arg1.foo`). The dot operator works on both
  pointer and non-pointer types.

  :topic: tracepoint_fields

  Args:
    expr (str): The expression to evaluate.

  Returns:
    px.TracingField: A materialized column pointer to use in output table definitions.
  )doc";

  inline static constexpr char kRetExprID[] = "RetExpr";
  inline static constexpr char kRetExprDocstring[] = R"doc(
  Specifies a function return value to trace.

  Extracts data from the function return value, as specified by the provided expression.
  Traceable types are the same as in `ArgExpr`. Return values are accessed by index
  (`$0` for the first return value, `$1` for the second return value, etc.).
  In Golang, the first index value is the number of arguments, excluding the receiver.
  For example, the return value for `fun Sum(a int, b int) int` is `$2`.
  Return values that are structs may be accessed using dotted notation, similar to `ArgExpr`,
  (e.g. `$0.foo`).

  :topic: tracepoint_fields

  Args:
    expr (str): The expression to evaluate.

  Returns:
    px.TracingField: A materialized column pointer to use in output table definitions.
  )doc";

  inline static constexpr char kFunctionLatencyID[] = "FunctionLatency";
  inline static constexpr char kFunctionLatencyDocstring[] = R"doc(
  Specifies a function latency to trace.

  Computes the function latency, from entry to return. The measured latency includes
  includes time spent in sub-calls.

  :topic: tracepoint_fields

  Returns:
    px.TracingField: A materialized column pointer to use in output table definitions.
  )doc";

  inline static constexpr char kUpsertTraceID[] = "UpsertTracepoint";
  inline static constexpr char kUpsertTracepointDocstring[] = R"doc(
  Deploys a tracepoint on a process and collects the traced data into a table.

  Deploys the tracepoint on the process (UPID) for the specified amount of time (TTL).
  The provided name uniquely identifies the tracepoint, and is used to manage the
  tracepoint (e.g. future calls to `UpsertTracepoint` or `DeleteTracepoint`.)
  A call to `UpsertTracepoint` on an existing tracepoint resets the TTL, but
  otherwise has no effect. A call to `UpsertTracepoint` on an existing tracepoint
  with a different tracepoint function will fail. UpsertTracepoint automatically
  creates a table with the provided name should it not exist; if the table exists
  but has a different schema, the deployment will fail.

  :topic: pixie_state_management

  Args:
    name (str): The name of the tracepoint. Should be unique with the probe_fn.
    table_name (str): The table name to write the results. The table is created
      if it does not exist. The table schema must match if the table does exist.
    probe_fn (px.ProbeFn): The tracepoint function.
    target (Union[px.UPID,px.SharedObject]): The process or shared object to trace as specified by unique Vizier PID.
    ttl (px.Duration): The length of time that a tracepoint will stay alive, after
      which it will be removed.
  )doc";

  inline static constexpr char kDeleteTracepointID[] = "DeleteTracepoint";
  inline static constexpr char kDeleteTracepointDocstring[] = R"doc(
  Deletes a tracepoint.

  Deletes the tracepoint with the provided name, should it exist.

  :topic: pixie_state_management

  Args:
    name (str): The name of the tracepoint.
  )doc";

  inline static constexpr char kGoProbeTraceDefinition[] = "goprobe";
  inline static constexpr char kGoProbeDocstring[] = R"doc(
  Decorates a tracepoint definition of a Go function.

  Specifies the decorated function as a goprobe tracepoint on the `trace_fn`
  name.

  :topic: tracepoint_decorator

  Args:
    trace_fn (str): The Go func to trace. Format is `<package_name>.<func_name>`.

  Returns:
    Func: The wrapped probe function.
  )doc";

  inline static constexpr char kSharedObjectID[] = "SharedObject";
  inline static constexpr char kSharedObjectDocstring[] = R"doc(
  Defines a shared object target for Tracepoints.

  :topic: tracepoint_fields

  Args:
    name (str): The name of the shared object.
    upid (px.UPID): A process which loads the shared object.

  Returns:
    SharedObject: A pointer to the SharedObject that can be passed as a target
    to UpsertTracepoint.
  )doc";

 protected:
  explicit TraceModule(MutationsIR* mutations_ir, ASTVisitor* ast_visitor)
      : QLObject(TraceModuleType, ast_visitor), mutations_ir_(mutations_ir) {}
  Status Init();

 private:
  MutationsIR* mutations_ir_;
};

class ProbeHandler {
 public:
  /**
   * @brief ProbeHandler is the handler for the @px.probe decorator. I find the structure of
   * decorators very confusing, but they are basically deeply nested functions. For
   * the probe() decorator., the equivalent function would look like this:
   * ```
   * def probe(fn_name, binary):
   *     def decorator_probes(func):
   *        def wrapper():
   *            pxtrace.StartProbe(fn_name, binary)
   *            r = func()
   *            pxtrace.EndProbe()
   *            # Returns the function return value.
   *            return r
   *        # Returns the wrapper function that will be called in place of func().
   *        return wrapper
   *    # Returns the decorator
   *    return decorator_probes
   * ```
   *
   * and then is called like the following
   * ```
   * @px.probe(...)
   * def probe_http():
   *    return [{"latency": pxtrace.FunctionLatency()},{"return": pxtrace.Return(0)}]
   * ```
   *
   * The AST Visitor will first call probes() with the arguments passed in then will
   *
   *
   * Whenever the decorator around a func, wrapper() replaces func() as the
   *
   * @param mutations_ir
   * @param ast
   * @param args
   * @param visitor
   * @return StatusOr<QLObjectPtr>
   */
  static StatusOr<QLObjectPtr> Probe(MutationsIR* mutations_ir,
                                     stirling::dynamic_tracing::ir::shared::Language language,
                                     const pypa::AstPtr& ast, const ParsedArgs& args,
                                     ASTVisitor* visitor);
  static StatusOr<QLObjectPtr> Decorator(MutationsIR* mutations_ir,
                                         stirling::dynamic_tracing::ir::shared::Language language,
                                         const std::string& function_name, const pypa::AstPtr& ast,
                                         const ParsedArgs& args, ASTVisitor* visitor);
  static StatusOr<QLObjectPtr> Wrapper(MutationsIR* mutations_ir,
                                       stirling::dynamic_tracing::ir::shared::Language language,
                                       const std::string& function_name,
                                       const std::shared_ptr<FuncObject> func_obj,
                                       const pypa::AstPtr& ast, const ParsedArgs& args,
                                       ASTVisitor* visitor);
};

/**
 * @brief Implements the pxtrace.Argument() logic.
 *
 */
class ArgumentHandler {
 public:
  static StatusOr<QLObjectPtr> Eval(MutationsIR* mutations_ir, const pypa::AstPtr& ast,
                                    const ParsedArgs& args, ASTVisitor* visitor);
};

}  // namespace compiler
}  // namespace planner
}  // namespace carnot
}  // namespace pl
