
#ifndef ___codeslayer_marshal_MARSHAL_H__
#define ___codeslayer_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,STRING (codeslayer_marshal.list:1) */
extern void _codeslayer_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* VOID:STRING,INT,STRING,INT (codeslayer_marshal.list:2) */
extern void _codeslayer_marshal_VOID__STRING_INT_STRING_INT (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

/* VOID:BOOLEAN,BOOLEAN (codeslayer_marshal.list:3) */
extern void _codeslayer_marshal_VOID__BOOLEAN_BOOLEAN (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* VOID:STRING,STRING,BOOLEAN,BOOLEAN,BOOLEAN (codeslayer_marshal.list:4) */
extern void _codeslayer_marshal_VOID__STRING_STRING_BOOLEAN_BOOLEAN_BOOLEAN (GClosure     *closure,
                                                                             GValue       *return_value,
                                                                             guint         n_param_values,
                                                                             const GValue *param_values,
                                                                             gpointer      invocation_hint,
                                                                             gpointer      marshal_data);

G_END_DECLS

#endif /* ___codeslayer_marshal_MARSHAL_H__ */

